#include "Items/BlackoutDropItem.h"

#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/BlackoutLog.h"
#include "Data/BOConsumableData.h"
#include "Framework/BlackoutPlayerState.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "EngineUtils.h"

ABlackoutDropItem::ABlackoutDropItem()
{
	// 기본 틱은 비활성화하여 CPU 연산을 아낍니다.
	PrimaryActorTick.bCanEverTick = false;

	// 멀티플레이어 리플리케이션 설정
	bReplicates = true;
	SetReplicateMovement(true);

	// 상호작용 감지 영역 초기화
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	SetRootComponent(InteractionSphere);
	InteractionSphere->InitSphereRadius(100.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 외관 스태틱 메시 초기화
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(InteractionSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 3D 월드 플로팅 위젯 컴포넌트 초기화
	InteractionWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidget"));
	InteractionWidget->SetupAttachment(InteractionSphere);
	InteractionWidget->SetWidgetSpace(EWidgetSpace::Screen); // 화면 공간 2D 투영 방식
	InteractionWidget->SetDrawAtDesiredSize(true);

	// 이펙트 출력을 위한 나이아가라 컴포넌트 직접 탑재 및 기본 비활성화 처리
	RewardEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RewardEffectComponent"));
	RewardEffectComponent->SetupAttachment(InteractionSphere);
	RewardEffectComponent->bAutoActivate = false;
}

void ABlackoutDropItem::BeginPlay()
{
	Super::BeginPlay();

	// 생성/BeginPlay 시점에 비주얼 상태 동기화
	UpdateRewardVisual();

	// 새로 스폰된 아이템이 플레이어 근처에 있을 수 있으므로, 즉시 포커스 스캔을 강제합니다.
	NotifyLocalPlayerInteractableAvailable();
}

bool ABlackoutDropItem::CanInteract_Implementation(AActor* Interactor) const
{
	// 이미 획득 진행 중이거나 비활성 상태면 획득 불가
	if (bIsCollected)
	{
		return false;
	}

	ABlackoutPlayerState* PlayerState = nullptr;
	if (!TryResolvePlayerState(Interactor, PlayerState))
	{
		return false;
	}

	// 획득 시 꽉 찬 자원이면 상호작용을 차단하여 낭비를 막음
	const UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();
	if (DropItemType == EBlackoutDropItemType::PrimaryAmmo)
	{
		// 주무기 탄약 충전이 필요한 상황인지 검사
		if (ASC)
		{
			return true;
		}
	}
	else if (DropItemType == EBlackoutDropItemType::SecondaryAmmo)
	{
		if (ASC)
		{
			return true;
		}
	}
	else if (DropItemType == EBlackoutDropItemType::Consumable)
	{
		// 블러드 루트나 굴 혈청 중 하나라도 인벤토리 여유가 남아있으면 획득 가능
		bool bHasSpace = false;
		if (BloodRootData)
		{
			const int32 Current = PlayerState->GetConsumableCount(BloodRootData->ConsumableTag);
			if (Current < BloodRootData->MaxCount)
			{
				bHasSpace = true;
			}
		}
		if (GulSerumData)
		{
			const int32 Current = PlayerState->GetConsumableCount(GulSerumData->ConsumableTag);
			if (Current < GulSerumData->MaxCount)
			{
				bHasSpace = true;
			}
		}
		return bHasSpace;
	}

	return false;
}

void ABlackoutDropItem::OnInteract_Implementation(AActor* Interactor)
{
	// 상호작용 판정 및 지급은 서버 권한을 기준으로 합니다.
	if (!HasAuthority())
	{
		BO_LOG_CORE(Warning, TEXT("ABlackoutDropItem::OnInteract: 서버가 아닌 클라이언트에서 호출되었습니다."));
		return;
	}

	if (bIsCollected)
	{
		return;
	}

	ABlackoutPlayerState* PlayerState = nullptr;
	if (!TryResolvePlayerState(Interactor, PlayerState))
	{
		BO_LOG_CORE(Warning, TEXT("ABlackoutDropItem::OnInteract: 유효한 PlayerState를 찾지 못했습니다."));
		return;
	}

	// 획득 시작 (중복 획득 차단)
	bIsCollected = true;

	// 비주얼 및 물리 차단 (즉각 반응성 보장)
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);
	UpdateRewardVisual();

	// 클라이언트로의 즉각 복제를 즉시 강제 (Relevancy가 끊기기 전 도달 보장)
	ForceNetUpdate();

	// 지급 처리
	if (DropItemType == EBlackoutDropItemType::PrimaryAmmo)
	{
		UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();
		if (ASC)
		{
			const float CurrentReserve = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute());
			
			float MaxReserve = 0.0f;
			if (ABlackoutPlayerCharacter* PC = Cast<ABlackoutPlayerCharacter>(Interactor))
			{
				if (const UBlackoutCombatComponent* CombatComp = PC->GetCombatComponent())
				{
					if (const ABOFirearm* Primary = CombatComp->GetPrimaryFirearm())
					{
						MaxReserve = static_cast<float>(Primary->GetMaxReserveAmmo());
					}
				}
			}

			// 안전장치(Fallback): 무기 인스턴스를 찾지 못했다면 기존 한 탄창 값 기준으로 연산
			if (MaxReserve <= 0.0f)
			{
				const float MaxClip = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute());
				MaxReserve = MaxClip > 0.0f ? MaxClip * 5.0f : 150.0f;
			}

			// 최대 예비 탄약량 * 비율
			const float ChargeAmount = FMath::CeilToFloat(MaxReserve * PrimaryAmmoChargeRatio);

			ASC->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute(), CurrentReserve + ChargeAmount);

			BO_LOG_CORE(Log, TEXT("주무기 탄약 충전 완료: Player=%s, +%f (MaxReserve=%f, %f -> %f)"),
				*PlayerState->GetPlayerName(), ChargeAmount, MaxReserve, CurrentReserve, CurrentReserve + ChargeAmount);
		}
	}
	else if (DropItemType == EBlackoutDropItemType::SecondaryAmmo)
	{
		UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();
		if (ASC)
		{
			const float CurrentReserve = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute());

			float MaxReserve = 0.0f;
			if (ABlackoutPlayerCharacter* PC = Cast<ABlackoutPlayerCharacter>(Interactor))
			{
				if (const UBlackoutCombatComponent* CombatComp = PC->GetCombatComponent())
				{
					if (const ABOFirearm* Secondary = CombatComp->GetSecondaryFirearm())
					{
						MaxReserve = static_cast<float>(Secondary->GetMaxReserveAmmo());
					}
				}
			}

			// 안전장치(Fallback)
			if (MaxReserve <= 0.0f)
			{
				const float MaxClip = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute());
				MaxReserve = MaxClip > 0.0f ? MaxClip * 5.0f : 75.0f;
			}

			// 최대 예비 탄약량 * 비율
			const float ChargeAmount = FMath::CeilToFloat(MaxReserve * SecondaryAmmoChargeRatio);

			ASC->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute(), CurrentReserve + ChargeAmount);

			BO_LOG_CORE(Log, TEXT("보조무기 탄약 충전 완료: Player=%s, +%f (MaxReserve=%f, %f -> %f)"),
				*PlayerState->GetPlayerName(), ChargeAmount, MaxReserve, CurrentReserve, CurrentReserve + ChargeAmount);
		}
	}
	else if (DropItemType == EBlackoutDropItemType::Consumable)
	{
		// 소모품 무작위 지급 처리 (50:50 확률 분기)
		UBOConsumableData* SelectedData = nullptr;
		if (BloodRootData && GulSerumData)
		{
			const float RandValue = FMath::FRand();
			SelectedData = (RandValue < 0.5f) ? BloodRootData : GulSerumData;

			// 스마트 보정: 만약 선택된 소모품 수량이 이미 꽉 찬 상태라면 나머지 빈자리 소모품으로 자동 지급
			const int32 SelectedCount = PlayerState->GetConsumableCount(SelectedData->ConsumableTag);
			if (SelectedCount >= SelectedData->MaxCount)
			{
				UBOConsumableData* FallbackData = (SelectedData == BloodRootData) ? GulSerumData : BloodRootData;
				const int32 FallbackCount = PlayerState->GetConsumableCount(FallbackData->ConsumableTag);
				if (FallbackCount < FallbackData->MaxCount)
				{
					SelectedData = FallbackData;
				}
			}
		}
		else if (BloodRootData)
		{
			SelectedData = BloodRootData;
		}
		else if (GulSerumData)
		{
			SelectedData = GulSerumData;
		}

		if (SelectedData)
		{
			const int32 CurrentCount = PlayerState->GetConsumableCount(SelectedData->ConsumableTag);
			const int32 NewCount = FMath::Clamp(CurrentCount + 1, 0, SelectedData->MaxCount);
			PlayerState->SetConsumableCount(SelectedData->ConsumableTag, NewCount);

			BO_LOG_CORE(Log, TEXT("소모품 지급 완료: Player=%s, Item=%s (%d -> %d)"),
				*PlayerState->GetPlayerName(), *SelectedData->ConsumableTag.ToString(), CurrentCount, NewCount);
		}
	}

	// 획득이 완전히 종료되었으므로 파괴하지 않고 풀링 서브시스템으로 반환하여 재사용
	// 단, 클라이언트가 획득/가시성 복제 패킷을 충분히 수신할 시간을 확보하기 위해 0.05초간 미세 지연 후 반환합니다.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ReturnToPoolTimerHandle,
			FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				if (UWorld* LocalWorld = GetWorld())
				{
					if (UBlackoutPoolSubsystem* PoolSubsystem = LocalWorld->GetSubsystem<UBlackoutPoolSubsystem>())
					{
						PoolSubsystem->ReturnToPool(this);
					}
				}
			}),
			0.05f,
			false
		);
	}
}

FText ABlackoutDropItem::GetInteractionPrompt_Implementation() const
{
	// 기획 상 플로팅 UI만 표시하므로 단축키 외 특수 텍스트는 비워둡니다.
	return FText::GetEmpty();
}

void ABlackoutDropItem::OnSpawnFromPool_Implementation()
{
	bIsCollected = false;

	// 물리 콜리전 및 비주얼 복구
	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	SetActorHiddenInGame(false);

	if (InteractionWidget)
	{
		InteractionWidget->SetHiddenInGame(false);
		InteractionWidget->SetVisibility(true, true);
	}

	// 나이아가라 컴포넌트 가시성 동기화
	UpdateRewardVisual();

	// 풀 재사용 경로에서도 인접 로컬 플레이어가 즉시 위젯을 잡도록 트리거
	NotifyLocalPlayerInteractableAvailable();

	// 바닥에서 영구 방치되어 자원이 낭비되는 것을 차단하기 위해 30초 수명 타이머 시동 (서버 전용)
	if (HasAuthority() && LifeTime > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			LifeTimeTimerHandle,
			this,
			&ABlackoutDropItem::OnLifeTimeExpired,
			LifeTime,
			false
		);
	}
}

void ABlackoutDropItem::OnReturnToPool_Implementation()
{
	// 타이머 해제
	GetWorldTimerManager().ClearTimer(LifeTimeTimerHandle);
	GetWorldTimerManager().ClearTimer(ReturnToPoolTimerHandle);

	// 물리 및 비주얼 차단
	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	SetActorHiddenInGame(true);

	if (InteractionWidget)
	{
		InteractionWidget->SetHiddenInGame(true);
		InteractionWidget->SetVisibility(false, true);
	}

	// 나이아가라 이펙트 즉시 해제 및 초기화
	if (RewardEffectComponent)
	{
		RewardEffectComponent->DeactivateImmediate();
		RewardEffectComponent->SetAsset(nullptr);
	}
}

void ABlackoutDropItem::SetDropItemType(EBlackoutDropItemType NewType)
{
	DropItemType = NewType;
	UpdateRewardVisual();
}

void ABlackoutDropItem::InitializeDropReward(EBlackoutDropItemType NewType, float NewSupplyRatio)
{
	DropItemType = NewType;
	if (NewType == EBlackoutDropItemType::PrimaryAmmo)
	{
		PrimaryAmmoChargeRatio = NewSupplyRatio;
	}
	else if (NewType == EBlackoutDropItemType::SecondaryAmmo)
	{
		SecondaryAmmoChargeRatio = NewSupplyRatio;
	}
	// 소모품일 경우 SupplyRatio가 쓰이지 않으므로 무시합니다.

	// 보상 타입 주입 완료 시점에 비주얼 동기화
	UpdateRewardVisual();
}

void ABlackoutDropItem::SnapToGround(AActor* IgnoreActor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TraceStart = CurrentLocation + FVector(0.0f, 0.0f, FMath::Max(GroundSnapTraceUpDistance, 0.0f));
	const FVector TraceEnd = CurrentLocation - FVector(0.0f, 0.0f, FMath::Max(GroundSnapTraceDownDistance, 0.0f));

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutDropItemSnapToGround), false, this);
	QueryParams.AddIgnoredActor(this);
	if (IgnoreActor)
	{
		QueryParams.AddIgnoredActor(IgnoreActor);
	}

	// 모든 Pawn 계열을 무시 대상으로 추가하여 폰 충돌체에 의해 아이템이 공중에 걸리는 것을 방지
	for (APawn* Pawn : TActorRange<APawn>(World))
	{
		if (Pawn)
		{
			QueryParams.AddIgnoredActor(Pawn);
		}
	}

	if (!World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams)
		|| !GroundHit.bBlockingHit)
	{
		BO_LOG_CORE(Verbose, TEXT("ABlackoutDropItem::SnapToGround: 바닥 탐색 실패. Drop=%s Location=%s"),
			*GetNameSafe(this),
			*CurrentLocation.ToString());
		return;
	}

	float MeshBottomOffsetFromActor = 0.0f;
	if (PickupMesh && PickupMesh->IsRegistered())
	{
		const FBoxSphereBounds MeshBounds = PickupMesh->Bounds;
		if (MeshBounds.BoxExtent.Z > KINDA_SMALL_NUMBER)
		{
			MeshBottomOffsetFromActor = (MeshBounds.Origin.Z - MeshBounds.BoxExtent.Z) - CurrentLocation.Z;
		}
	}

	FVector SnappedLocation = CurrentLocation;
	SnappedLocation.Z = GroundHit.ImpactPoint.Z + GroundSnapClearance - MeshBottomOffsetFromActor;
	SetActorLocation(SnappedLocation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ABlackoutDropItem::OnLifeTimeExpired()
{
	if (!HasAuthority())
	{
		return;
	}

	// 획득 시도를 한 적이 없다면 풀 서브시스템을 통해 유휴 풀로 자진 귀환
	UWorld* World = GetWorld();
	if (World)
	{
		UBlackoutPoolSubsystem* PoolSubsystem = World->GetSubsystem<UBlackoutPoolSubsystem>();
		if (PoolSubsystem)
		{
			PoolSubsystem->ReturnToPool(this);
			BO_LOG_CORE(Verbose, TEXT("드롭 아이템 수명 만료로 풀 귀환: Name=%s"), *GetName());
		}
	}
}

void ABlackoutDropItem::NotifyLocalPlayerInteractableAvailable()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ABlackoutPlayerCharacter> It(World); It; ++It)
	{
		ABlackoutPlayerCharacter* PlayerCharacter = *It;
		if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
		{
			PlayerCharacter->ForceRefreshFocusedInteractable();
			break;
		}
	}
}

bool ABlackoutDropItem::TryResolvePlayerState(AActor* Interactor, ABlackoutPlayerState*& OutPlayerState) const
{
	OutPlayerState = nullptr;
	if (!Interactor)
	{
		return false;
	}

	const APawn* InteractorPawn = Cast<APawn>(Interactor);
	OutPlayerState = InteractorPawn ? InteractorPawn->GetPlayerState<ABlackoutPlayerState>() : nullptr;
	return OutPlayerState != nullptr;
}

void ABlackoutDropItem::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);

	if (InteractionWidget)
	{
		InteractionWidget->SetHiddenInGame(bNewHidden);
	}

	// 가시성 변화에 맞춰 이펙트 상태도 완벽히 동기화
	UpdateRewardVisual();
}

void ABlackoutDropItem::PostNetReceive()
{
	Super::PostNetReceive();

	const bool bIsHiddenOnClient = IsHidden();

	// movement 복제 등 빈번한 PostNetReceive마다 위젯/Niagara가 토글되는 것을 방지.
	// 실제로 bHidden 값이 변했을 때만 동기화 작업을 수행합니다.
	if (bHasCachedReplicatedHidden && bLastReplicatedHidden == bIsHiddenOnClient)
	{
		return;
	}

	bLastReplicatedHidden = bIsHiddenOnClient;
	bHasCachedReplicatedHidden = true;

	if (InteractionWidget)
	{
		InteractionWidget->SetHiddenInGame(bIsHiddenOnClient);
		InteractionWidget->SetVisibility(!bIsHiddenOnClient, true);
	}

	// 클라이언트 측 가시성 복제에 따른 나이아가라 이펙트 완벽 동기화
	UpdateRewardVisual();

	// hidden→visible 전환 시 로컬 플레이어 포커스 스캔 즉시 트리거
	if (!bIsHiddenOnClient)
	{
		NotifyLocalPlayerInteractableAvailable();
	}
}

void ABlackoutDropItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutDropItem, bIsCollected);
	DOREPLIFETIME(ABlackoutDropItem, DropItemType);
}

void ABlackoutDropItem::OnRep_DropItemType()
{
	UpdateRewardVisual();
}

void ABlackoutDropItem::OnRep_bIsCollected()
{
	if (bIsCollected)
	{
		if (InteractionSphere)
		{
			InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		SetActorHiddenInGame(true);

		if (InteractionWidget)
		{
			InteractionWidget->SetHiddenInGame(true);
			InteractionWidget->SetVisibility(false, true);
		}

		UpdateRewardVisual();
	}
	else
	{
		// 풀 재사용 시 true→false 전환 시점에 클라이언트 측 콜리전·가시성·위젯을 원복합니다.
		// PostNetReceive(bHidden 변경) 의존을 줄여, 패킷 순서/타이밍과 무관하게 위젯이 살아나도록 보장합니다.
		if (InteractionSphere)
		{
			InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		SetActorHiddenInGame(false);

		if (InteractionWidget)
		{
			InteractionWidget->SetHiddenInGame(false);
			InteractionWidget->SetVisibility(true, true);
		}

		UpdateRewardVisual();

		// 풀 재사용으로 다시 활성화된 시점에 인접 플레이어가 위젯을 즉시 잡도록 강제 스캔
		NotifyLocalPlayerInteractableAvailable();
	}
}

void ABlackoutDropItem::UpdateRewardVisual()
{
	// 가시성을 판단하는 종합 체크 (게임 내에 숨겨졌거나 이미 획득한 상태라면 비가시 처리)
	const bool bShouldBeVisible = !IsHidden() && !bIsCollected;

	if (!bShouldBeVisible)
	{
		// 단순 hide 경로에서는 에셋을 비우지 않고 비활성화만 합니다.
		// SetAsset(nullptr)은 풀 반환(OnReturnToPool)에서만 호출하여, 매 PostNetReceive마다 에셋이
		// nullptr ↔ TargetFX로 교체되는 잔상/리셋을 방지합니다.
		if (RewardEffectComponent)
		{
			RewardEffectComponent->DeactivateImmediate();
		}
		return;
	}

	// 활성화 시 보상 타입에 대응하는 이펙트 선택
	UNiagaraSystem* TargetFX = nullptr;
	switch (DropItemType)
	{
	case EBlackoutDropItemType::PrimaryAmmo:
		TargetFX = PrimaryAmmoFX;
		break;
	case EBlackoutDropItemType::SecondaryAmmo:
		TargetFX = SecondaryAmmoFX;
		break;
	case EBlackoutDropItemType::Consumable:
		TargetFX = ConsumableFX;
		break;
	}

	if (RewardEffectComponent)
	{
		// 이미 설정된 에셋과 다르면 교체
		if (RewardEffectComponent->GetAsset() != TargetFX)
		{
			RewardEffectComponent->DeactivateImmediate();
			RewardEffectComponent->SetAsset(TargetFX);
		}

		if (TargetFX)
		{
			if (!RewardEffectComponent->IsActive())
			{
				RewardEffectComponent->Activate(true);
			}
		}
		else
		{
			RewardEffectComponent->DeactivateImmediate();
		}
	}
}
