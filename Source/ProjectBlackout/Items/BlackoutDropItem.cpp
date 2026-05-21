#include "Items/BlackoutDropItem.h"

#include "Components/SphereComponent.h"
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
}

void ABlackoutDropItem::BeginPlay()
{
	Super::BeginPlay();
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
			// 예비 탄약이 최대 클립 등 기획적 가이드에 따른 추가 확장을 감안하여, 일단 항상 획득할 수 있도록 하되
			// 필요 시 현재 예비탄 카운트를 검사하여 한도 내에서만 획득하게 조절 가능합니다.
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

	// 비주얼 및 물리 차단
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorHiddenInGame(true);

	// 지급 처리
	if (DropItemType == EBlackoutDropItemType::PrimaryAmmo)
	{
		UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();
		if (ASC)
		{
			const float MaxClip = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute());
			const float CurrentReserve = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute());

			// 주무기 충전량 연산: 최대 탄창 * 비율 (최대 탄창 값이 유효하지 않으면 30발 고정 보충)
			const float ChargeAmount = MaxClip > 0.0f ? FMath::CeilToFloat(MaxClip * PrimaryAmmoChargeRatio) : 30.0f;

			ASC->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute(), CurrentReserve + ChargeAmount);

			BO_LOG_CORE(Log, TEXT("주무기 탄약 충전 완료: Player=%s, +%f (%f -> %f)"),
				*PlayerState->GetPlayerName(), ChargeAmount, CurrentReserve, CurrentReserve + ChargeAmount);
		}
	}
	else if (DropItemType == EBlackoutDropItemType::SecondaryAmmo)
	{
		UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent();
		if (ASC)
		{
			const float MaxClip = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute());
			const float CurrentReserve = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute());

			// 보조무기 충전량 연산: 최대 탄창 * 비율 (최대 탄창 값이 유효하지 않으면 15발 고정 보충)
			const float ChargeAmount = MaxClip > 0.0f ? FMath::CeilToFloat(MaxClip * SecondaryAmmoChargeRatio) : 15.0f;

			ASC->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute(), CurrentReserve + ChargeAmount);

			BO_LOG_CORE(Log, TEXT("보조무기 탄약 충전 완료: Player=%s, +%f (%f -> %f)"),
				*PlayerState->GetPlayerName(), ChargeAmount, CurrentReserve, CurrentReserve + ChargeAmount);
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
	UWorld* World = GetWorld();
	if (World)
	{
		UBlackoutPoolSubsystem* PoolSubsystem = World->GetSubsystem<UBlackoutPoolSubsystem>();
		if (PoolSubsystem)
		{
			PoolSubsystem->ReturnToPool(this);
		}
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

	// 물리 및 비주얼 차단
	if (InteractionSphere)
	{
		InteractionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	SetActorHiddenInGame(true);
}

void ABlackoutDropItem::SetDropItemType(EBlackoutDropItemType NewType)
{
	DropItemType = NewType;
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
