#include "BlackoutPlayerCharacter.h"
#include "AbilitySystemGlobals.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Characters/BlackoutPlayerMovementComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Components/BlackoutImpactIndicatorComponent.h"
#include "Data/BOCharacterData.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Framework/BlackoutBattleGameMode.h"
#include "Framework/BlackoutGraphicsUserSettings.h"
#include "Framework/BlackoutPlayerState.h"
#include "AbilitySystemInterface.h"
#include "GAS/Abilities/Player/BlackoutGA_Revive.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/BlackoutInteractable.h"
#include "GameplayCueManager.h"
#include "BlackoutLog.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "Items/BlackoutDropItem.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SpotLightComponent.h"

ABlackoutPlayerCharacter::ABlackoutPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UBlackoutPlayerMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// TPS 카메라 셋업
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 350.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetUsingAbsoluteRotation(true);

	// 네트워크 위치 보정으로 캡슐이 순간 튀어도 카메라는 부드럽게 따라가도록 함 (시각적 jitter 흡수).
	// 회전 lag 은 켜지 않음 — TPS 조준 반응성 우선.
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.0f;
	SpringArm->CameraLagMaxDistance = 50.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	CombatComponent = CreateDefaultSubobject<UBlackoutCombatComponent>(TEXT("CombatComponent"));
	ImpactIndicatorComponent = CreateDefaultSubobject<UBlackoutImpactIndicatorComponent>(TEXT("ImpactIndicatorComponent"));
	ImpactIndicatorComponent->Initialize(CombatComponent);

	// TPS: 컨트롤러 회전은 카메라에만 적용하고, 기본 이동 회전은 CharacterMovement가 담당
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	// 기본 이동 속도 초기화 (Transient 변수의 안전장치용 기본값 설정)
	DefaultMaxWalkSpeed = 600.f;
	AimMaxWalkSpeed = 420.f;
	DownedMaxWalkSpeed = 150.f;

	// 플래시 라이트 컴포넌트 셋업
	FlashlightComponent = CreateDefaultSubobject<USpotLightComponent>(TEXT("FlashlightComponent"));
	FlashlightComponent->SetupAttachment(GetMesh()); // 블루프린트에서 원하는 소켓에 부착할 수 있도록 설정
	FlashlightComponent->SetVisibility(false);       // 기본적으로 꺼진 상태
	
	// SpotLight 기본값 설정 (에디터 상에서 수정 가능)
	FlashlightComponent->Intensity = 100000.f;        // 광원 세기 (Lumen 환경을 감안하여 10만으로 상향)
	FlashlightComponent->OuterConeAngle = 35.f;      // 외각 원뿔각
	FlashlightComponent->InnerConeAngle = 15.f;      // 내각 원뿔각
	FlashlightComponent->AttenuationRadius = 2500.f; // 감쇠 반경 (그림자 최적화를 위한 적정 반경 설정)
}

void ABlackoutPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CacheAimDefaults();
	UpdateAimMovementMode();

	// 플래시 라이트를 지정된 소켓에 동적 부착 (블루프린트 디폴트 서브오브젝트 소켓 수정 이슈 해결)
	if (FlashlightComponent && GetMesh())
	{
		FlashlightComponent->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FlashlightAttachSocketName);
	}
}

void ABlackoutPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlackoutPlayerCharacter, ReplicatedAimOffset, COND_SkipOwner);
	DOREPLIFETIME(ABlackoutPlayerCharacter, bIsReviveInteractionActive);
	DOREPLIFETIME(ABlackoutPlayerCharacter, DownedDeathServerEndTimeSeconds);
	DOREPLIFETIME(ABlackoutPlayerCharacter, DownedDeathPausedRemainingTime);
	DOREPLIFETIME(ABlackoutPlayerCharacter, bDownedDeathTimerPaused);
	DOREPLIFETIME(ABlackoutPlayerCharacter, ReviveServerStartTimeSeconds);
	DOREPLIFETIME(ABlackoutPlayerCharacter, ReviveDuration);
	DOREPLIFETIME(ABlackoutPlayerCharacter, bIsFlashlightOn);
}

void ABlackoutPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// aim 모드  틱 로컬 플레이어 카메라 갱신 
	if (!IsLocallyControlled())
	{
		return;
	}

	UpdateFocusedInteractable(DeltaSeconds);
	UpdateAimCamera(DeltaSeconds);
}


void ABlackoutPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (MoveAction)
	{
		// 회피시 입력했던 방향을 기억하기 위한 바인딩 
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlackoutPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABlackoutPlayerCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ABlackoutPlayerCharacter::Move);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABlackoutPlayerCharacter::Look);
	}

	if (MouseLookAction)
	{
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ABlackoutPlayerCharacter::Look);
	}

	if (ToggleFlashlightAction)
	{
		EnhancedInputComponent->BindAction(ToggleFlashlightAction, ETriggerEvent::Started, this, &ABlackoutPlayerCharacter::ToggleFlashlight);
	}
}
void ABlackoutPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Server: InitAbilityActorInfo
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetPlayerState()))
	{
		AbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(
			ASCInterface->GetAbilitySystemComponent());

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
			BindDownedStateTagEvent();
			ApplyReplicatedReviveInteractionStateTag();

			// 초기 스탯 및 어빌리티 부여
			InitializeAttributes();

			if (CharacterData)
			{
				if (ABlackoutPlayerState* BlackoutPlayerState = GetPlayerState<ABlackoutPlayerState>())
				{
					BlackoutPlayerState->InitializeConsumablesFromCharacterData(CharacterData);
				}

				AbilitySystemComponent->GiveDefaultAbilities(CharacterData->GrantedAbilities);
				AbilitySystemComponent->GiveConsumableAbilities(CharacterData->ConsumableSlots);

				if (CombatComponent)
				{
					CombatComponent->InitializeLoadoutFromCharacterData(CharacterData);
				}

				BO_LOG_GAS(Log, "Abilities granted to %s", *GetName());
			}
		}
	}
}

void ABlackoutPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetPlayerState()))
	{
		AbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(
			ASCInterface->GetAbilitySystemComponent());

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
			BindDownedStateTagEvent();
			ApplyReplicatedReviveInteractionStateTag();

			// 클라이언트에서도 어트리뷰트 초기화
			InitializeAttributes();
		}
	}
}

void ABlackoutPlayerCharacter::ApplyPull(const FPullData& PullData)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;
   
	MoveComp->Velocity += PullData.PullDirection * PullData.PullStrength * PullData.DeltaTime;
}

void ABlackoutPlayerCharacter::Server_RequestDebugSelfDamage_Implementation(float DamageAmount)
{
	if (!HasAuthority())
	{
		return;
	}

	if (DamageAmount <= 0.f)
	{
		BO_LOG_GAS(Warning, "Server_RequestDebugSelfDamage failed: DamageAmount가 0 이하임");
		return;
	}

	if (!AbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "Server_RequestDebugSelfDamage failed: AbilitySystemComponent가 비어 있음");
		return;
	}

	if (!DebugSelfDamageEffect)
	{
		BO_LOG_GAS(Warning, "Server_RequestDebugSelfDamage failed: DebugSelfDamageEffect가 비어 있음");
		return;
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DebugSelfDamageEffect, 1.f, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		BO_LOG_GAS(Warning, "Server_RequestDebugSelfDamage failed: Damage Spec 생성에 실패함");
		return;
	}

	SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, DamageAmount);

	const bool bApplied = ApplyIncomingDamageSpec(SpecHandle, NAME_None);
	BO_LOG_GAS(Log,
		"Server_RequestDebugSelfDamage: Target=%s Damage=%.1f Applied=%s",
		*GetName(),
		DamageAmount,
		bApplied ? TEXT("true") : TEXT("false"));
}

void ABlackoutPlayerCharacter::Server_SetAimOffset_Implementation(FVector2D NewAimOffset)
{
	ReplicatedAimOffset = FVector2D(
		FMath::Clamp(NewAimOffset.X, -180.f, 180.f),
		FMath::Clamp(NewAimOffset.Y, -90.f, 90.f));
}

FVector ABlackoutPlayerCharacter::GetFocusedInteractablePromptWorldLocation() const
{
	AActor* TargetActor = FocusedInteractableActor.Get();
	if (!IsValid(TargetActor))
	{
		return FVector::ZeroVector;
	}

	FVector BoundsOrigin = FVector::ZeroVector;
	FVector BoundsExtent = FVector::ZeroVector;
	// 콜리전이 있는 컴포넌트만 bounds 계산에 포함합니다.
	// false로 넘기면 Niagara emit particle 등 비콜리전 컴포넌트의 가변 bounds가 함께 잡혀
	// 프롬프트 위치가 시뮬레이션 진행에 따라 화면 위쪽으로 끌려가는 버그가 발생합니다.
	TargetActor->GetActorBounds(true, BoundsOrigin, BoundsExtent);
	return BoundsOrigin + FVector(0.0f, 0.0f, BoundsExtent.Z + InteractionPromptHeightOffset);
}

bool ABlackoutPlayerCharacter::TryInteractWithFocusedActor()
{
	RefreshFocusedInteractableActor();

	AActor* TargetActor = FocusedInteractableActor.Get();
	if (!IsValidFocusedInteractable(TargetActor))
	{
		return false;
	}

	// 로컬 체감 레이턴시 0ms 극대화를 위한 선제 숨김/비활성화 처리 (클라이언트 및 서버 로컬 공통)
	if (ABlackoutDropItem* DropItem = Cast<ABlackoutDropItem>(TargetActor))
	{
		DropItem->SetActorHiddenInGame(true);
		if (UWidgetComponent* Widget = DropItem->FindComponentByClass<UWidgetComponent>())
		{
			Widget->SetHiddenInGame(true);
			Widget->SetVisibility(false, true);
		}
		if (USphereComponent* Sphere = DropItem->FindComponentByClass<USphereComponent>())
		{
			Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	if (HasAuthority())
	{
		IBlackoutInteractable::Execute_OnInteract(TargetActor, this);
	}
	else
	{
		Server_InteractWithActor(TargetActor);
	}

	// 상호작용 실행 즉시 포커스를 해제하여 중복 요청 및 위젯 잔상 갱신 대기를 미연에 방지
	FocusedInteractableActor = nullptr;

	return true;
}

bool ABlackoutPlayerCharacter::HasNearbyReviveTarget() const
{
	if (IsDead() || IsDowned())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float ReviveRange = UBlackoutGA_Revive::GetReviveRangeForActor(this);
	const float ReviveRangeSquared = FMath::Square(ReviveRange);

	for (TActorIterator<ABlackoutPlayerCharacter> It(World); It; ++It)
	{
		ABlackoutPlayerCharacter* Candidate = *It;
		if (!Candidate || Candidate == this || Candidate->IsDead() || !Candidate->IsDowned())
		{
			continue;
		}

		if (FVector::DistSquared(Candidate->GetActorLocation(), GetActorLocation()) <= ReviveRangeSquared)
		{
			return true;
		}
	}

	return false;
}

void ABlackoutPlayerCharacter::UpdateFocusedInteractable(float DeltaSeconds)
{
	if (IsDead() || IsDowned())
	{
		FocusedInteractableActor = nullptr;
		InteractionScanElapsed = 0.0f;
		return;
	}

	InteractionScanElapsed += DeltaSeconds;
	if (InteractionScanInterval > 0.0f && InteractionScanElapsed < InteractionScanInterval)
	{
		return;
	}

	InteractionScanElapsed = 0.0f;
	RefreshFocusedInteractableActor();
}

void ABlackoutPlayerCharacter::ForceRefreshFocusedInteractable()
{
	// 스캔 누적 시간을 초기화하여, 다음 틱이 아닌 즉시 시점에 새 상호작용 대상이 잡히도록 합니다.
	InteractionScanElapsed = 0.0f;
	RefreshFocusedInteractableActor();
}

void ABlackoutPlayerCharacter::RefreshFocusedInteractableActor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		FocusedInteractableActor = nullptr;
		return;
	}

	TArray<FOverlapResult> OverlapResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutFocusedInteractable), false, this);
	const bool bHasOverlap = World->OverlapMultiByObjectType(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(InteractionSearchRadius),
		QueryParams);

	if (!bHasOverlap)
	{
		FocusedInteractableActor = nullptr;
		return;
	}

	AActor* BestCandidate = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* CandidateActor = OverlapResult.GetActor();
		if (!IsValidFocusedInteractable(CandidateActor))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(CandidateActor->GetActorLocation(), GetActorLocation());
		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestCandidate = CandidateActor;
		}
	}

	FocusedInteractableActor = BestCandidate;
}

bool ABlackoutPlayerCharacter::IsValidFocusedInteractable(AActor* CandidateActor) const
{
	if (!IsValid(CandidateActor) || CandidateActor == this)
	{
		return false;
	}

	if (!CandidateActor->GetClass()->ImplementsInterface(UBlackoutInteractable::StaticClass()))
	{
		return false;
	}

	return IBlackoutInteractable::Execute_CanInteract(CandidateActor, const_cast<ABlackoutPlayerCharacter*>(this));
}

void ABlackoutPlayerCharacter::Server_InteractWithActor_Implementation(AActor* TargetActor)
{
	if (!IsValidFocusedInteractable(TargetActor))
	{
		BO_LOG_CORE(Verbose, "상호작용 무시: 서버에서 유효한 대상이 아닙니다. Player=%s Target=%s", *GetName(), *GetNameSafe(TargetActor));
		return;
	}

	IBlackoutInteractable::Execute_OnInteract(TargetActor, this);
}


// 회피 몽타주 재생은 GAS 표준 PlayMontageAndWait → ASC::PlayMontage 경로로 처리됩니다.

void ABlackoutPlayerCharacter::Multicast_PlayHitReactMontage_Implementation(UAnimMontage* Montage, float PlayRate,
	bool bAllowMovementDuringHitReact)
{
	PlayHitReactMontage(Montage, PlayRate, bAllowMovementDuringHitReact);
}

void ABlackoutPlayerCharacter::Multicast_PlayFireMontage_Implementation(UAnimMontage* Montage, float PlayRate, bool bRestartIfPlaying)
{
	PlayFireMontage(Montage, PlayRate, bRestartIfPlaying);
}

void ABlackoutPlayerCharacter::Multicast_PlayReloadMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayReloadMontage(Montage, PlayRate);
}

void ABlackoutPlayerCharacter::Multicast_StopFireMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
	StopFireMontage(Montage, BlendOutTime);
}

// PlayDodgeMontage 본체는 TDD §4.1 v2 에서 폐기되었습니다.
// GA_Dodge 는 UAbilityTask_PlayMontageAndWait 로 직접 재생하며,
// 회피 진행 상태는 SetDodgeMontagePlaying setter 로 외부에 알립니다.

bool ABlackoutPlayerCharacter::PlayFireMontage(UAnimMontage* Montage, float PlayRate, bool bRestartIfPlaying)
{
	if (!Montage)
	{
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayFireMontage failed: MeshComponent가 비어 있음");
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayFireMontage failed: AnimInstance가 비어 있음");
		return false;
	}

	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		if (!bRestartIfPlaying)
		{
			BO_LOG_GAS(Verbose, "PlayFireMontage skipped: 이미 같은 사격 몽타주가 재생 중임");
			return true;
		}

		AnimInstance->Montage_Stop(0.03f, Montage);
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);

	return PlayResult > 0.f;
}

bool ABlackoutPlayerCharacter::StopFireMontage(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage)
	{
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(Montage))
	{
		return false;
	}

	AnimInstance->Montage_Stop(BlendOutTime, Montage);
	return true;
}

bool ABlackoutPlayerCharacter::PlayReloadMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayReloadMontage failed: Montage가 비어 있음");
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayReloadMontage failed: MeshComponent가 비어 있음");
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayReloadMontage failed: AnimInstance가 비어 있음");
		return false;
	}

	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		BO_LOG_GAS(Verbose, "PlayReloadMontage skipped: 이미 같은 장전 몽타주가 재생 중임");
		return true;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	BO_LOG_GAS(Log,
		"PlayReloadMontage result=%.2f Local=%s Authority=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	return PlayResult > 0.f;
}

UAnimMontage* ABlackoutPlayerCharacter::GetFireMontageForTag(FGameplayTag FireAnimTag) const
{
	for (const FBlackoutFireMontageEntry& Entry : FireMontageEntries)
	{
		if (Entry.FireAnimTag == FireAnimTag && Entry.Montage)
		{
			return Entry.Montage;
		}
	}

	return nullptr;
}

UAnimMontage* ABlackoutPlayerCharacter::GetReloadMontageForTag(FGameplayTag ReloadAnimTag, bool bIsTwoHanded) const
{
	for (const FBlackoutReloadMontageEntry& Entry : ReloadMontageEntries)
	{
		if (Entry.ReloadAnimTag == ReloadAnimTag && Entry.Montage)
		{
			return Entry.Montage;
		}
	}

	return bIsTwoHanded ? ReloadFallbackMontage2R : ReloadFallbackMontage1R;
}

void ABlackoutPlayerCharacter::Multicast_StopReloadMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
	StopReloadMontage(Montage, BlendOutTime);
}

bool ABlackoutPlayerCharacter::StopReloadMontage(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage)
	{
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(Montage))
	{
		return false;
	}

	AnimInstance->Montage_Stop(BlendOutTime, Montage);
	return true;
}

bool ABlackoutPlayerCharacter::PlayHitReactMontage(UAnimMontage* Montage, float PlayRate, bool bAllowMovementDuringHitReact)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayHitReactMontage failed: Montage가 비어 있음");
		bIsHitReactMontagePlaying = false;
		bCanMoveDuringHitReact = false;
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayHitReactMontage failed: MeshComponent가 비어 있음");
		bIsHitReactMontagePlaying = false;
		bCanMoveDuringHitReact = false;
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayHitReactMontage failed: AnimInstance가 비어 있음");
		bIsHitReactMontagePlaying = false;
		bCanMoveDuringHitReact = false;
		return false;
	}

	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		bIsHitReactMontagePlaying = true;
		bCanMoveDuringHitReact = bAllowMovementDuringHitReact;
		BO_LOG_GAS(Verbose,
			"PlayHitReactMontage skipped: 이미 같은 히트 몽타주가 재생 중임 AllowMove=%s",
			bCanMoveDuringHitReact ? TEXT("true") : TEXT("false"));
		return true;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	if (PlayResult > 0.f)
	{
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ABlackoutPlayerCharacter::HandleHitReactMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
		bIsHitReactMontagePlaying = true;
		bCanMoveDuringHitReact = bAllowMovementDuringHitReact;
	}
	else
	{
		bIsHitReactMontagePlaying = false;
		bCanMoveDuringHitReact = false;
	}

	BO_LOG_GAS(Log,
		"PlayHitReactMontage result=%.2f Local=%s Authority=%s AllowMove=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		bCanMoveDuringHitReact ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	return PlayResult > 0.f;
}

void ABlackoutPlayerCharacter::Multicast_PlayWeaponSwapMontage_Implementation(FGameplayTag TargetWeaponSlotTag, float PlayRate)
{
	PlayWeaponSwapMontage(TargetWeaponSlotTag, PlayRate);
}

void ABlackoutPlayerCharacter::Multicast_ExecuteWeaponGameplayCue_Implementation(FGameplayTag CueTag, FGameplayCueParameters CueParameters, bool bSkipLocallyControlled)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (bSkipLocallyControlled && IsLocallyControlled() && GetNetMode() != NM_Standalone)
	{
		return;
	}

	if (!CueTag.IsValid())
	{
		BO_LOG_GAS(Warning, "Multicast_ExecuteWeaponGameplayCue skipped: CueTag가 유효하지 않음");
		return;
	}

	if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
	{
		CueManager->HandleGameplayCue(this, CueTag, EGameplayCueEvent::Executed, CueParameters);
		return;
	}

	BO_LOG_GAS(Error, "Multicast_ExecuteWeaponGameplayCue failed: GameplayCueManager가 유효하지 않음 (Cue=%s)", *CueTag.ToString());
}

void ABlackoutPlayerCharacter::Multicast_ExecuteWeaponGameplayCueBatch_Implementation(const TArray<FBlackoutWeaponGameplayCueEntry>& CueEntries, bool bSkipLocallyControlled)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (bSkipLocallyControlled && IsLocallyControlled() && GetNetMode() != NM_Standalone)
	{
		return;
	}

	if (CueEntries.IsEmpty())
	{
		return;
	}

	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	if (!CueManager)
	{
		BO_LOG_GAS(Error, "Multicast_ExecuteWeaponGameplayCueBatch failed: GameplayCueManager가 유효하지 않음 (Count=%d)", CueEntries.Num());
		return;
	}

	for (const FBlackoutWeaponGameplayCueEntry& CueEntry : CueEntries)
	{
		if (!CueEntry.CueTag.IsValid())
		{
			BO_LOG_GAS(Warning, "Multicast_ExecuteWeaponGameplayCueBatch skipped: CueTag가 유효하지 않음");
			continue;
		}

		CueManager->HandleGameplayCue(this, CueEntry.CueTag, EGameplayCueEvent::Executed, CueEntry.CueParameters);
	}
}

bool ABlackoutPlayerCharacter::PlayWeaponSwapMontage(FGameplayTag TargetWeaponSlotTag, float PlayRate)
{
	UAnimMontage* Montage = GetWeaponSwapMontageForSlot(TargetWeaponSlotTag);
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayWeaponSwapMontage failed: 슬롯 %s 에 대응하는 몽타주가 비어 있음", *TargetWeaponSlotTag.ToString());
		bIsWeaponSwapMontagePlaying = false;
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayWeaponSwapMontage failed: MeshComponent가 비어 있음");
		bIsWeaponSwapMontagePlaying = false;
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayWeaponSwapMontage failed: AnimInstance가 비어 있음");
		bIsWeaponSwapMontagePlaying = false;
		return false;
	}

	// 로컬 예측 재생 후 멀티캐스트가 도착해도 같은 몽타주를 다시 시작하지 않도록 방지
	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		bIsWeaponSwapMontagePlaying = true;
		return true;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	if (PlayResult > 0.f)
	{
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ABlackoutPlayerCharacter::HandleWeaponSwapMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
		bIsWeaponSwapMontagePlaying = true;
		return true;
	}

	bIsWeaponSwapMontagePlaying = false;
	return false;
}

UAnimMontage* ABlackoutPlayerCharacter::GetWeaponSwapMontageForSlot(FGameplayTag TargetWeaponSlotTag) const
{
	if (TargetWeaponSlotTag == BlackoutGameplayTags::Weapon_Primary)
	{
		return EquipPrimaryMontage;
	}

	if (TargetWeaponSlotTag == BlackoutGameplayTags::Weapon_Secondary)
	{
		return EquipSecondaryMontage;
	}

	return nullptr;
}

// 근접 콤보 몽타주 RPC/헬퍼는 TDD §4.1 v2 에서 폐기되었습니다.
// 재생: UAbilityTask_PlayMontageAndWait → ASC::PlayMontage → FRepAnimMontageInfo 자동 복제
// 섹션 점프: 서버에서 ASC::CurrentMontageJumpToSection 호출 → RepAnimMontageInfo 로 클라이언트 자동 따라잡음

void ABlackoutPlayerCharacter::Client_JumpMontageToSection_Implementation(UAnimMontage* Montage, FName SectionName, bool bApplyControlYaw, float ControlYawDegrees)
{
	if (!Montage || SectionName == NAME_None || HasAuthority() || !IsLocallyControlled())
	{
		return;
	}

	if (bApplyControlYaw)
	{
		SetActorRotation(FRotator(0.f, ControlYawDegrees, 0.f));
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(Montage))
	{
		BO_LOG_GAS(Verbose,
			"Client_JumpMontageToSection skipped: Montage=%s Section=%s",
			*GetNameSafe(Montage),
			*SectionName.ToString());
		return;
	}

	// 서버가 승인한 섹션 전환만 오너 클라이언트의 예측 몽타주 인스턴스에 반영합니다.
	AnimInstance->Montage_JumpToSection(SectionName, Montage);
}

void ABlackoutPlayerCharacter::Multicast_SyncDodgeChainRestart_Implementation(UAnimMontage* Montage, FName SectionName, float ServerYawDegrees)
{
	if (!Montage || SectionName == NAME_None || HasAuthority() || IsLocallyControlled())
	{
		return;
	}

	// 시뮬레이트 프록시에서 루트 모션 방향과 몽타주 위치가 한 프레임 어긋나지 않도록 함께 맞춥니다.
	SetActorRotation(FRotator(0.f, ServerYawDegrees, 0.f));

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "Multicast_SyncDodgeChainRestart failed: AnimInstance가 비어 있음");
		return;
	}

	if (!AnimInstance->Montage_IsPlaying(Montage))
	{
		const float PlayResult = PlayAnimMontage(Montage, 1.f);
		if (PlayResult <= 0.f)
		{
			BO_LOG_GAS(Warning,
				"Multicast_SyncDodgeChainRestart failed: 회피 몽타주 재생 실패 Montage=%s",
				*GetNameSafe(Montage));
			return;
		}
	}

	AnimInstance->Montage_JumpToSection(SectionName, Montage);
}

void ABlackoutPlayerCharacter::Multicast_PlayConsumableMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		return;
	}

	if (IsLocallyControlled() && !HasAuthority())
	{
		// 로컬 예측 GA에서 이미 재생한 소유 클라이언트는 서버 multicast로 몽타주를 다시 시작하지 않습니다.
		return;
	}

	PlayAnimMontage(Montage, PlayRate);
}

void ABlackoutPlayerCharacter::Multicast_StopConsumableMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	if (AnimInstance && AnimInstance->Montage_IsPlaying(Montage))
	{
		AnimInstance->Montage_Stop(BlendOutTime, Montage);
	}
}

void ABlackoutPlayerCharacter::Client_BeginAbilityMovementOverride_Implementation(float SpeedMultiplier, bool bStopMovementImmediately, bool bAddLockedTag)
{
	if (!IsLocallyControlled() || HasAuthority())
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		BO_LOG_GAS(Warning, "Client_BeginAbilityMovementOverride failed: MovementComponent가 비어 있음");
		return;
	}

	if (CachedAbilityOverrideMaxWalkSpeed <= 0.0f)
	{
		CachedAbilityOverrideMaxWalkSpeed = MoveComp->MaxWalkSpeed;
	}

	if (bStopMovementImmediately)
	{
		MoveComp->StopMovementImmediately();
	}

	MoveComp->MaxWalkSpeed = CachedAbilityOverrideMaxWalkSpeed * FMath::Clamp(SpeedMultiplier, 0.0f, 1.0f);

	if (bAddLockedTag && AbilitySystemComponent && !bAppliedLocalAbilityLockedTag)
	{
		// 서버 태그 복제 전까지 소유 클라이언트 입력을 같은 프레임 규칙으로 막습니다.
		AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Locked);
		bAppliedLocalAbilityLockedTag = true;
	}
}

void ABlackoutPlayerCharacter::Client_EndAbilityMovementOverride_Implementation()
{
	if (!IsLocallyControlled() || HasAuthority())
	{
		return;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (CachedAbilityOverrideMaxWalkSpeed > 0.0f)
		{
			MoveComp->MaxWalkSpeed = CachedAbilityOverrideMaxWalkSpeed;
		}
	}

	if (AbilitySystemComponent && bAppliedLocalAbilityLockedTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Locked);
	}

	CachedAbilityOverrideMaxWalkSpeed = 0.0f;
	bAppliedLocalAbilityLockedTag = false;
}

void ABlackoutPlayerCharacter::CommitPendingWeaponSwap()
{
	if (CombatComponent)
	{
		CombatComponent->CommitPendingWeaponSwap();
	}
}

// HandleDodgeMontageEnded 는 TDD §4.1 v2 에서 폐기되었습니다.
// 회피 진행 플래그는 GA_Dodge 가 SetDodgeMontagePlaying setter 로 직접 관리합니다.

void ABlackoutPlayerCharacter::HandleHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsHitReactMontagePlaying = false;
	bCanMoveDuringHitReact = false;
	BO_LOG_GAS(Log,
		"Hit react montage ended: Interrupted=%s Montage=%s",
		bInterrupted ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));
}

void ABlackoutPlayerCharacter::HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsDeathMontagePlaying = false;
	BO_LOG_GAS(Log,
		"Death montage ended: Interrupted=%s Montage=%s",
		bInterrupted ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));
}

void ABlackoutPlayerCharacter::HandleWeaponSwapMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsWeaponSwapMontagePlaying = false;

	if (CombatComponent)
	{
		CombatComponent->HandleWeaponSwapMontageEnded(bInterrupted);
	}

	BO_LOG_GAS(Log,
		"Weapon swap montage ended: Interrupted=%s Montage=%s",
		bInterrupted ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));
}

void ABlackoutPlayerCharacter::HandleAimStateChanged(bool bNewAiming)
{
	ApplyAimMovementMode(bNewAiming);
}

bool ABlackoutPlayerCharacter::IsReviving() const
{
	return AbilitySystemComponent
		&& AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Reviving);
}

bool ABlackoutPlayerCharacter::IsBeingRevived() const
{
	return AbilitySystemComponent
		? AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_BeingRevived)
		: bIsReviveInteractionActive;
}

bool ABlackoutPlayerCharacter::TryBeginReviveInteraction(ABlackoutPlayerCharacter* Reviver, float InReviveDuration)
{
	if (!HasAuthority() || !Reviver || IsDead() || !IsDowned())
	{
		return false;
	}

	if (IsBeingRevived())
	{
		if (!ActiveReviver.IsValid())
		{
			SetBeingRevivedStateActive(false);
			BroadcastReviveInteractionStateChanged();
		}
		else
		{
			return ActiveReviver.Get() == Reviver;
		}
	}

	// 부활 진행률을 다운된 본인 및 다른 클라이언트가 계산할 수 있도록 서버 시작 시각/총 지속 시간을 복제합니다.
	const UWorld* World = GetWorld();
	ReviveServerStartTimeSeconds = World ? World->GetTimeSeconds() : 0.0f;
	ReviveDuration = FMath::Max(0.0f, InReviveDuration);

	SetBeingRevivedStateActive(true);
	Reviver->SetRevivingStateActive(true);
	ActiveReviver = Reviver;
	BroadcastReviveInteractionStateChanged();
	return true;
}

void ABlackoutPlayerCharacter::EndReviveInteraction(ABlackoutPlayerCharacter* Reviver)
{
	if (!HasAuthority() || !IsBeingRevived())
	{
		return;
	}

	if (ActiveReviver.IsValid() && Reviver && ActiveReviver.Get() != Reviver)
	{
		return;
	}

	if (ABlackoutPlayerCharacter* CurrentReviver = ActiveReviver.Get())
	{
		CurrentReviver->SetRevivingStateActive(false);
	}

	SetBeingRevivedStateActive(false);
	ActiveReviver = nullptr;

	// 부활 진행률 복제 필드를 초기화해 HUD가 즉시 부활 진행 표시를 종료하도록 합니다.
	ReviveServerStartTimeSeconds = 0.0f;
	ReviveDuration = 0.0f;

	BroadcastReviveInteractionStateChanged();
}

void ABlackoutPlayerCharacter::OnRep_ReviveInteractionActive()
{
	ApplyReplicatedReviveInteractionStateTag();
	BroadcastReviveInteractionStateChanged();
}

void ABlackoutPlayerCharacter::BroadcastReviveInteractionStateChanged()
{
	OnReviveInteractionStateChangedNative.Broadcast(this, IsBeingRevived());
}

UAnimMontage* ABlackoutPlayerCharacter::SelectDirectionalHitReactMontage(UAnimMontage* FrontMontage, UAnimMontage* BackMontage,
	UAnimMontage* DefaultMontage, bool bIsBackHitReact) const
{
	if (bIsBackHitReact)
	{
		if (BackMontage)
		{
			return BackMontage;
		}

		if (DefaultMontage)
		{
			return DefaultMontage;
		}

		return FrontMontage;
	}

	if (FrontMontage)
	{
		return FrontMontage;
	}

	if (DefaultMontage)
	{
		return DefaultMontage;
	}

	return BackMontage;
}

bool ABlackoutPlayerCharacter::IsBackHitReact(const FVector& DamageSourceLocation) const
{
	const FVector ToDamageSource = (DamageSourceLocation - GetActorLocation()).GetSafeNormal2D();
	if (ToDamageSource.IsNearlyZero())
	{
		return false;
	}

	const FVector Forward2D = GetActorForwardVector().GetSafeNormal2D();
	if (Forward2D.IsNearlyZero())
	{
		return false;
	}

	const float ForwardDot = FVector::DotProduct(Forward2D, ToDamageSource);
	return ForwardDot <= BackHitReactDirectionDotThreshold;
}

UAnimMontage* ABlackoutPlayerCharacter::SelectHitReactMontage(float AppliedDamage, bool bIsBackHitReact) const
{
	const bool bUseHeavyHitReact = AppliedDamage > HeavyHitReactDamageThreshold;
	const bool bIsAimingHitReact = !bUseHeavyHitReact
		&& CombatComponent
		&& CombatComponent->IsAiming()
		&& !IsDowned()
		&& !IsDead();

	// Light/Heavy 전용 몽타주가 비어 있어도 기존 단일 몽타주 자산으로 안전하게 폴백합니다.
	if (bUseHeavyHitReact)
	{
		if (UAnimMontage* SelectedMontage = SelectDirectionalHitReactMontage(
			HeavyFrontHitReactMontage, HeavyBackHitReactMontage, HeavyHitReactMontage, bIsBackHitReact))
		{
			return SelectedMontage;
		}

		if (HitReactMontage)
		{
			return HitReactMontage;
		}

		return SelectDirectionalHitReactMontage(
			LightFrontHitReactMontage, LightBackHitReactMontage, LightHitReactMontage, bIsBackHitReact);
	}

	if (bIsAimingHitReact)
	{
		if (UAnimMontage* SelectedMontage = SelectDirectionalHitReactMontage(
			AimedLightFrontHitReactMontage, AimedLightBackHitReactMontage, AimedLightHitReactMontage, bIsBackHitReact))
		{
			return SelectedMontage;
		}

		if (UAnimMontage* SelectedMontage = SelectDirectionalHitReactMontage(
			LightFrontHitReactMontage, LightBackHitReactMontage, LightHitReactMontage, bIsBackHitReact))
		{
			return SelectedMontage;
		}

		if (HitReactMontage)
		{
			return HitReactMontage;
		}

		return HeavyHitReactMontage;
	}

	if (UAnimMontage* SelectedMontage = SelectDirectionalHitReactMontage(
		LightFrontHitReactMontage, LightBackHitReactMontage, LightHitReactMontage, bIsBackHitReact))
	{
		return SelectedMontage;
	}

	if (HitReactMontage)
	{
		return HitReactMontage;
	}

	return SelectDirectionalHitReactMontage(
		HeavyFrontHitReactMontage, HeavyBackHitReactMontage, HeavyHitReactMontage, bIsBackHitReact);
}

void ABlackoutPlayerCharacter::OnHitReact(float AppliedDamage, const FVector& DamageSourceLocation)
{
	Super::OnHitReact(AppliedDamage, DamageSourceLocation);

	if (!HasAuthority())
	{
		return;
	}

	const bool bIsBackHitReact = IsBackHitReact(DamageSourceLocation);
	UAnimMontage* SelectedHitReactMontage = SelectHitReactMontage(AppliedDamage, bIsBackHitReact);
	const bool bIsAimingLightHitReact = AppliedDamage <= HeavyHitReactDamageThreshold
		&& CombatComponent
		&& CombatComponent->IsAiming()
		&& !IsDowned()
		&& !IsDead();
	if (!SelectedHitReactMontage)
	{
		BO_LOG_GAS(Verbose, "OnHitReact skipped: 사용할 히트 몽타주가 비어 있음 Damage=%.1f", AppliedDamage);
		return;
	}

	BO_LOG_GAS(Log,
		"OnHitReact selected montage: Damage=%.1f Threshold=%.1f Type=%s Direction=%s Aiming=%s AllowMove=%s Montage=%s",
		AppliedDamage,
		HeavyHitReactDamageThreshold,
		AppliedDamage > HeavyHitReactDamageThreshold
			? TEXT("Heavy")
			: (bIsAimingLightHitReact ? TEXT("AimedLight") : TEXT("Light")),
		bIsBackHitReact ? TEXT("Back") : TEXT("Front"),
		bIsAimingLightHitReact ? TEXT("true") : TEXT("false"),
		bIsAimingLightHitReact ? TEXT("true") : TEXT("false"),
		*GetNameSafe(SelectedHitReactMontage));

	Multicast_PlayHitReactMontage(SelectedHitReactMontage, 1.f, bIsAimingLightHitReact);
}

void ABlackoutPlayerCharacter::HandleDownedStateChanged(bool bWasDowned, bool bIsCurrentlyDowned)
{
	if (bIsCurrentlyDowned)
	{
		ApplyDownedStateLocally();
		return;
	}

	ClearDownedStateLocally();

	if (bWasDowned && !IsDead() && ReviveMontage)
	{
		PlayReviveMontage(ReviveMontage, 1.f);
		return;
	}

	if (bWasDowned && !IsDead())
	{
		RestoreWeaponVisibilityAfterRevive();
	}
}

void ABlackoutPlayerCharacter::ApplyDownedStateLocally()
{
	bIsHitReactMontagePlaying = false;
	bCanMoveDuringHitReact = false;
	bIsDodgeMontagePlaying = false;
	bIsWeaponSwapMontagePlaying = false;
	bIsReviveMontagePlaying = false;
	bIsDeathMontagePlaying = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReviveWeaponRestoreTimerHandle);
	}

	if (CombatComponent)
	{
		CombatComponent->StopFire();
		CombatComponent->HandlePrimaryActionReleased();
		CombatComponent->StopAim();
		CombatComponent->BeginEquippedWeaponHiddenOverride();
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->MaxWalkSpeed = DownedMaxWalkSpeed;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = false;
	}

	UpdateAimMovementMode();
}

void ABlackoutPlayerCharacter::ClearDownedStateLocally()
{
	if (IsDead())
	{
		return;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	UpdateAimMovementMode();
}

void ABlackoutPlayerCharacter::OnDowned()
{
	if (IsDead() || IsDowned())
	{
		return;
	}

	Super::OnDowned();

	EndReviveInteraction(nullptr);
	StartDownedDeathTimer();

	if (DownedEnterMontage)
	{
		Multicast_PlayDownedEnterMontage(DownedEnterMontage, 1.f);
	}
}

bool ABlackoutPlayerCharacter::CanEnterDownedState() const
{
	if (IsDead() || IsDowned())
	{
		return false;
	}

	// 1인 솔로 플레이 시에는 부활시켜 줄 아군이 없으므로 다운 상태에 진입할 수 없게 차단하여 즉시 완전 사망(OnDeath)하도록 유도합니다.
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			if (GS->PlayerArray.Num() <= 1)
			{
				return false;
			}
		}
	}

	return true;
}

void ABlackoutPlayerCharacter::OnDeath()
{
	if (IsDead())
	{
		return;
	}

	Super::OnDeath();

	ClearDownedDeathTimer();
	EndReviveInteraction(nullptr);

	bIsHitReactMontagePlaying = false;
	bCanMoveDuringHitReact = false;
	bIsDodgeMontagePlaying = false;
	bIsWeaponSwapMontagePlaying = false;
	bIsReviveMontagePlaying = false;
	bIsDeathMontagePlaying = false;

	if (CombatComponent)
	{
		CombatComponent->StopFire();
		CombatComponent->HandlePrimaryActionReleased();
		CombatComponent->StopAim();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReviveWeaponRestoreTimerHandle);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		// 사망 시 다른 캐릭터(Pawn)와 부딪히지 않고 통과하여 지나갈 수 있도록 충돌을 무시합니다.
		CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	if (UAnimMontage* SelectedDeathMontage = SelectDeathMontage())
	{
		Multicast_PlayDeathMontage(SelectedDeathMontage, 1.f);
	}
	else
	{
		BO_LOG_GAS(Warning, "OnDeath: 재생할 사망 몽타주가 설정되지 않았습니다. Player=%s", *GetNameSafe(this));
	}

	NotifyBattleGameModePlayerFullyDead();
}

void ABlackoutPlayerCharacter::Server_ReviveFromDowned_Implementation(float RevivedHealth)
{
	if (!HasAuthority() || !AbilitySystemComponent || !IsDowned() || IsDead())
	{
		return;
	}

	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	const float ClampedHealth = FMath::Clamp(RevivedHealth, 1.f, MaxHealth);

	ClearDownedDeathTimer();
	SetDownedStateActive(false);
	EndReviveInteraction(nullptr);
	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutBaseAttributeSet::GetHealthAttribute(), ClampedHealth);
	ScheduleWeaponVisibilityRestoreAfterRevive();
	BO_LOG_GAS(Log, "ReviveFromDowned: Target=%s Health=%.1f", *GetName(), ClampedHealth);
}

void ABlackoutPlayerCharacter::RestoreToFullState()
{
	if (!HasAuthority())
	{
		return;
	}

	ClearDownedDeathTimer();
	SetDeadStateActive(false);
	SetDownedStateActive(false);
	SetBeingRevivedStateActive(false);
	SetRevivingStateActive(false);
	ActiveReviver = nullptr;

	bIsHitReactMontagePlaying = false;
	bIsDodgeMontagePlaying = false;
	bIsWeaponSwapMontagePlaying = false;
	bIsReviveMontagePlaying = false;
	bIsDeathMontagePlaying = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReviveWeaponRestoreTimerHandle);
	}

	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
		{
			AnimInstance->StopAllMontages(0.1f);
		}
	}

	if (CombatComponent)
	{
		CombatComponent->StopFire();
		CombatComponent->HandlePrimaryActionReleased();
		CombatComponent->StopAim();
		CombatComponent->EndEquippedWeaponHiddenOverride();
	}

	if (AbilitySystemComponent)
	{
		const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
		if (MaxHealth <= 0.0f)
		{
			BO_LOG_GAS(Error, "PartyWipeRestart 복구 실패: MaxHealth가 0 이하입니다. Player=%s", *GetNameSafe(this));
		}
		else
		{
			AbilitySystemComponent->SetNumericAttributeBase(UBlackoutBaseAttributeSet::GetHealthAttribute(), MaxHealth);
		}
	}
	else
	{
		BO_LOG_GAS(Error, "PartyWipeRestart 복구 실패: AbilitySystemComponent가 비어 있습니다. Player=%s", *GetNameSafe(this));
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
		MoveComp->MaxWalkSpeed = DefaultMaxWalkSpeed;
		MoveComp->bOrientRotationToMovement = true;
		MoveComp->bUseControllerDesiredRotation = false;
	}

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		// 부활하여 복구될 때 캡슐의 Pawn 충돌 반응을 원래 상태(블록)로 원상복구합니다.
		CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
	}

	UpdateAimMovementMode();
	BO_LOG_GAS(Log, "PartyWipeRestart 복구 완료: Player=%s", *GetNameSafe(this));
}


UAnimMontage* ABlackoutPlayerCharacter::SelectDeathMontage() const
{
	TArray<UAnimMontage*> ValidMontages;
	for (const TObjectPtr<UAnimMontage>& Montage : DeathMontageVariants)
	{
		if (Montage.Get())
		{
			ValidMontages.Add(Montage.Get());
		}
	}

	if (ValidMontages.Num() > 0)
	{
		const int32 SelectedIndex = FMath::RandRange(0, ValidMontages.Num() - 1);
		return ValidMontages[SelectedIndex];
	}

	return nullptr;
}

bool ABlackoutPlayerCharacter::PlayDeathMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayDeathMontage failed: Montage가 비어 있음");
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayDeathMontage failed: MeshComponent가 비어 있음");
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayDeathMontage failed: AnimInstance가 비어 있음");
		return false;
	}

	// 클라이언트에서 사망 몽타주를 시작하는 즉시 로컬 캡슐 충돌 반응을 무시로 설정하여 레이턴시 오차를 보완합니다.
	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	AnimInstance->StopAllMontages(0.05f);

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	if (PlayResult > 0.f)
	{
		bIsDeathMontagePlaying = true;

		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ABlackoutPlayerCharacter::HandleDeathMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
	}

	BO_LOG_GAS(Log,
		"PlayDeathMontage result=%.2f Local=%s Authority=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	return PlayResult > 0.f;
}

void ABlackoutPlayerCharacter::Multicast_PlayDeathMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayDeathMontage(Montage, PlayRate);
}

bool ABlackoutPlayerCharacter::PlayDownedEnterMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayDownedEnterMontage failed: Montage가 비어 있음");
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayDownedEnterMontage failed: MeshComponent가 비어 있음");
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayDownedEnterMontage failed: AnimInstance가 비어 있음");
		return false;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	BO_LOG_GAS(Log,
		"PlayDownedEnterMontage result=%.2f Local=%s Authority=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	return PlayResult > 0.f;
}

void ABlackoutPlayerCharacter::Multicast_PlayDownedEnterMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayDownedEnterMontage(Montage, PlayRate);
}

bool ABlackoutPlayerCharacter::PlayReviveMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayReviveMontage failed: Montage가 비어 있음");
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayReviveMontage failed: MeshComponent가 비어 있음");
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayReviveMontage failed: AnimInstance가 비어 있음");
		return false;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	if (PlayResult > 0.f)
	{
		bIsReviveMontagePlaying = true;

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
		}

		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ABlackoutPlayerCharacter::HandleReviveMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
	}
	BO_LOG_GAS(Log,
		"PlayReviveMontage result=%.2f Local=%s Authority=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	return PlayResult > 0.f;
}

void ABlackoutPlayerCharacter::Multicast_PlayRevivePerformMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayRevivePerformMontage(Montage, PlayRate);
	
}

bool ABlackoutPlayerCharacter::PlayRevivePerformMontage(UAnimMontage* Montage, float PlayRate)
{
	
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayRevivePerformMontage failed: Montage가 비어 있음");
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayRevivePerformMontage failed: MeshComponent가 비어 있음");
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayRevivePerformMontage failed: AnimInstance가 비어 있음");
		return false;
	}

	// 로컬 예측 재생 후 멀티캐스트가 와도 같은 몽타주를 다시 시작하지 않도록 방지
	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		return true;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	BO_LOG_GAS(Log,
		"PlayRevivePerformMontage result=%.2f Local=%s Authority=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	return PlayResult > 0.f;
}

void ABlackoutPlayerCharacter::Multicast_StopRevivePerformMontage_Implementation(UAnimMontage* Montage,
	float BlendOutTime)
{
	StopRevivePerformMontage(Montage , BlendOutTime);
}

bool ABlackoutPlayerCharacter::StopRevivePerformMontage(UAnimMontage* Montage, float BlendOutTime)
{
	if (!Montage)
	{
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(Montage))
	{
		return false;
	}

	AnimInstance->Montage_Stop(BlendOutTime, Montage);
	return true;
}

void ABlackoutPlayerCharacter::Multicast_PlayReviveMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayReviveMontage(Montage, PlayRate);
}

void ABlackoutPlayerCharacter::RestoreWeaponVisibilityAfterRevive()
{
	if (CombatComponent)
	{
		CombatComponent->EndEquippedWeaponHiddenOverride();
	}
}

void ABlackoutPlayerCharacter::ScheduleWeaponVisibilityRestoreAfterRevive()
{
	if (!HasAuthority())
	{
		return;
	}

	if (!CombatComponent)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReviveWeaponRestoreTimerHandle);

		const float RestoreDelay = ReviveMontage ? FMath::Max(ReviveMontage->GetPlayLength(), 0.0f) : 0.0f;
		if (RestoreDelay <= 0.0f)
		{
			RestoreWeaponVisibilityAfterRevive();
			return;
		}

		World->GetTimerManager().SetTimer(
			ReviveWeaponRestoreTimerHandle,
			this,
			&ABlackoutPlayerCharacter::RestoreWeaponVisibilityAfterRevive,
			RestoreDelay,
			false);
	}
}

void ABlackoutPlayerCharacter::HandleReviveMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsReviveMontagePlaying = false;

	if (!IsDowned() && !IsDead())
	{
		RestoreWeaponVisibilityAfterRevive();
	}
}

void ABlackoutPlayerCharacter::StartDownedDeathTimer()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		BO_LOG_GAS(Error, "다운 사망 타이머 시작 실패: World가 비어 있습니다. Player=%s", *GetNameSafe(this));
		return;
	}

	const float SafeDuration = FMath::Max(1.0f, DownedDeathDuration);
	World->GetTimerManager().ClearTimer(DownedDeathTimerHandle);
	World->GetTimerManager().SetTimer(
		DownedDeathTimerHandle,
		this,
		&ABlackoutPlayerCharacter::HandleDownedDeathTimerExpired,
		SafeDuration,
		false);

	// 클라이언트가 남은 시간을 계산할 수 있도록 서버 월드 시간 기준 만료 시각을 복제합니다.
	DownedDeathServerEndTimeSeconds = World->GetTimeSeconds() + SafeDuration;
	DownedDeathPausedRemainingTime = 0.0f;
	bDownedDeathTimerPaused = false;

	BO_LOG_GAS(Log, "다운 사망 타이머 시작: Player=%s Duration=%.1f", *GetNameSafe(this), SafeDuration);
}

void ABlackoutPlayerCharacter::ClearDownedDeathTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DownedDeathTimerHandle);
	}

	// 복제 동기화 필드를 함께 초기화해 클라이언트 HUD가 잔여 시간을 0으로 인식하도록 합니다.
	if (HasAuthority())
	{
		DownedDeathServerEndTimeSeconds = 0.0f;
		DownedDeathPausedRemainingTime = 0.0f;
		bDownedDeathTimerPaused = false;
	}
}

void ABlackoutPlayerCharacter::PauseDownedDeathTimer()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !DownedDeathTimerHandle.IsValid())
	{
		return;
	}

	// 부활 시도 중에는 완전 사망 카운트다운을 멈춰 남은 시간을 유지합니다.
	const float RemainingAtPause = FMath::Max(0.0f, World->GetTimerManager().GetTimerRemaining(DownedDeathTimerHandle));
	World->GetTimerManager().PauseTimer(DownedDeathTimerHandle);

	DownedDeathPausedRemainingTime = RemainingAtPause;
	DownedDeathServerEndTimeSeconds = 0.0f;
	bDownedDeathTimerPaused = true;
}

void ABlackoutPlayerCharacter::ResumeDownedDeathTimer()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !DownedDeathTimerHandle.IsValid() || !bDownedDeathTimerPaused)
	{
		return;
	}

	// 부활이 취소되면 정지 시점의 남은 시간으로 사망 타이머를 재개합니다.
	World->GetTimerManager().UnPauseTimer(DownedDeathTimerHandle);

	const float NewRemaining = FMath::Max(0.0f, World->GetTimerManager().GetTimerRemaining(DownedDeathTimerHandle));
	DownedDeathServerEndTimeSeconds = World->GetTimeSeconds() + NewRemaining;
	DownedDeathPausedRemainingTime = 0.0f;
	bDownedDeathTimerPaused = false;
}

float ABlackoutPlayerCharacter::GetDownedDeathRemainingTime() const
{
	if (!IsDowned() || IsDead())
	{
		return 0.0f;
	}

	// 일시정지 중에는 복제된 정지 시점의 남은 시간을 그대로 사용합니다.
	if (bDownedDeathTimerPaused)
	{
		return FMath::Max(0.0f, DownedDeathPausedRemainingTime);
	}

	if (DownedDeathServerEndTimeSeconds <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	// 서버 월드 시간 기준으로 만료 시각과의 차이를 계산합니다.
	// 클라이언트에서는 GameState의 서버 동기화 시간을 사용하고, 서버에서는 World->GetTimeSeconds()와 동일합니다.
	float ServerNowSeconds = World->GetTimeSeconds();
	if (const AGameStateBase* GameStateBase = World->GetGameState())
	{
		ServerNowSeconds = GameStateBase->GetServerWorldTimeSeconds();
	}

	return FMath::Max(0.0f, DownedDeathServerEndTimeSeconds - ServerNowSeconds);
}

float ABlackoutPlayerCharacter::GetReviveRemainingTime() const
{
	if (!IsDowned() || IsDead() || !IsBeingRevived())
	{
		return 0.0f;
	}

	if (ReviveDuration <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	float ServerNowSeconds = World->GetTimeSeconds();
	if (const AGameStateBase* GameStateBase = World->GetGameState())
	{
		ServerNowSeconds = GameStateBase->GetServerWorldTimeSeconds();
	}

	const float Elapsed = FMath::Max(0.0f, ServerNowSeconds - ReviveServerStartTimeSeconds);
	return FMath::Clamp(ReviveDuration - Elapsed, 0.0f, ReviveDuration);
}

float ABlackoutPlayerCharacter::GetReviveProgressNormalized() const
{
	if (ReviveDuration <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	const float Remaining = GetReviveRemainingTime();
	return FMath::Clamp(1.0f - (Remaining / ReviveDuration), 0.0f, 1.0f);
}

void ABlackoutPlayerCharacter::HandleDownedDeathTimerExpired()
{
	if (!HasAuthority() || IsDead())
	{
		return;
	}

	if (!IsDowned())
	{
		BO_LOG_GAS(Warning, "다운 사망 타이머 만료 무시: 이미 다운 상태가 아닙니다. Player=%s", *GetNameSafe(this));
		return;
	}

	BO_LOG_GAS(Log, "다운 사망 타이머 만료: Player=%s", *GetNameSafe(this));
	OnDeath();
}

void ABlackoutPlayerCharacter::NotifyBattleGameModePlayerFullyDead()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		BO_LOG_GAS(Error, "완전 사망 알림 실패: World가 비어 있습니다. Player=%s", *GetNameSafe(this));
		return;
	}

	ABlackoutBattleGameMode* BattleGameMode = World->GetAuthGameMode<ABlackoutBattleGameMode>();
	if (!BattleGameMode)
	{
		BO_LOG_GAS(Warning, "완전 사망 알림 실패: BattleGameMode가 아닙니다. Player=%s", *GetNameSafe(this));
		return;
	}

	BattleGameMode->NotifyPlayerFullyDead(this);
}

void ABlackoutPlayerCharacter::SetRevivingStateActive(bool bNewReviving)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (bNewReviving)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Reviving))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Reviving);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Reviving);
	}
}

void ABlackoutPlayerCharacter::SetBeingRevivedStateActive(bool bNewBeingRevived)
{
	if (HasAuthority())
	{
		bIsReviveInteractionActive = bNewBeingRevived;

		// 서버 권위 사망 타이머를 부활 시도 시작 시 일시정지, 취소 시 재개합니다.
		if (bNewBeingRevived)
		{
			PauseDownedDeathTimer();
		}
		else
		{
			ResumeDownedDeathTimer();
		}
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	if (bNewBeingRevived)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_BeingRevived))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_BeingRevived);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_BeingRevived);
	}
}

void ABlackoutPlayerCharacter::ApplyReplicatedReviveInteractionStateTag()
{
	if (HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	if (bIsReviveInteractionActive)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_BeingRevived))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_BeingRevived);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_BeingRevived);
	}
}

void ABlackoutPlayerCharacter::CacheAimDefaults()
{
	if (SpringArm)
	{
		DefaultArmLength = SpringArm->TargetArmLength;
		DefaultSocketOffset = SpringArm->SocketOffset;
	}

	if (Camera)
	{
		DefaultFOV = Camera->FieldOfView;
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		DefaultMaxWalkSpeed = MoveComp->MaxWalkSpeed;
	}
}

void ABlackoutPlayerCharacter::UpdateAimCamera(float DeltaSeconds)
{
	if (!SpringArm || !Camera || !CombatComponent)
	{
		return;
	}

	const bool bIsAiming = CombatComponent->IsAiming() && !IsDowned() && !IsDead();

	const float TargetArmLength = bIsAiming ? AimArmLength : DefaultArmLength;
	const FVector TargetSocketOffset = bIsAiming ? AimSocketOffset : DefaultSocketOffset;
	const float TargetFOV = ResolveTargetCameraFOV(bIsAiming);

	
	//aim 모드 카메라 숄더에 고정 
	SpringArm->TargetArmLength = FMath::FInterpTo(
		SpringArm->TargetArmLength,
		TargetArmLength,
		DeltaSeconds,
		AimCameraInterpSpeed);

	SpringArm->SocketOffset = FMath::VInterpTo(
		SpringArm->SocketOffset,
		TargetSocketOffset,
		DeltaSeconds,
		AimCameraInterpSpeed);

	Camera->SetFieldOfView(FMath::FInterpTo(
		Camera->FieldOfView,
		TargetFOV,
		DeltaSeconds,
		AimCameraInterpSpeed));
}

float ABlackoutPlayerCharacter::ResolveTargetCameraFOV(bool bIsAiming) const
{
	if (bIsAiming)
	{
		return AimFOV;
	}

	if (bIsDodgeMontagePlaying)
	{
		return DodgeFOV;
	}

	if (bIsLocalSprintCameraActive)
	{
		return SprintFOV;
	}

	return DefaultFOV;
}

void ABlackoutPlayerCharacter::UpdateAimMovementMode()
{
	if (!CombatComponent)
	{
		return;
	}

	ApplyAimMovementMode(CombatComponent->IsAiming());
}

void ABlackoutPlayerCharacter::ApplyAimMovementMode(bool bIsAiming)
{
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (IsDowned())
		{
			MoveComp->bOrientRotationToMovement = true;
			MoveComp->bUseControllerDesiredRotation = false;
			MoveComp->MaxWalkSpeed = DownedMaxWalkSpeed;
			return;
		}

		MoveComp->bOrientRotationToMovement = !bIsAiming;
		MoveComp->bUseControllerDesiredRotation = bIsAiming;
		MoveComp->MaxWalkSpeed = bIsAiming ? AimMaxWalkSpeed : DefaultMaxWalkSpeed;
	}
}

void ABlackoutPlayerCharacter::InitializeAttributes()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (!DefaultAttributeEffect)
	{
		BO_LOG_GAS(Warning, "DefaultAttributeEffect is not set in %s", *GetName());
		return;
	}

	if (!CharacterData)
	{
		BO_LOG_GAS(Warning, "CharacterData is not set in %s", *GetName());
		return;
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1.f, ContextHandle);
	if (SpecHandle.IsValid())
	{
		// CharacterData의 값을 SetByCaller 태그를 통해 GE로 전달
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_MaxHealth, CharacterData->BaseMaxHealth);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Health, CharacterData->BaseMaxHealth);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_MaxStamina, CharacterData->BaseMaxStamina);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Stamina, CharacterData->BaseMaxStamina);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_MovementSpeed, CharacterData->BaseMovementSpeed);

		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		BO_LOG_GAS(Log, "Attributes initialized for %s using GE", *GetName());

		// UBOCharacterData에 설정된 이동 속도 데이터를 로컬 Transient 변수에 동적으로 로드합니다.
		DefaultMaxWalkSpeed = CharacterData->BaseMovementSpeed;
		AimMaxWalkSpeed = CharacterData->AimMovementSpeed;
		DownedMaxWalkSpeed = CharacterData->DownedMovementSpeed;

		// 조준 및 다운 상태를 확인하고 올바른 이동 속도로 즉시 업데이트합니다.
		UpdateAimMovementMode();
	}
}


#pragma region InputSetup

void ABlackoutPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
	//* 입력방향 기억 
	CachedMoveInput = MovementVector;
	
	DoMove(MovementVector.X, MovementVector.Y);
}

void ABlackoutPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ABlackoutPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController() == nullptr)
	{
		return;
	}

	if (IsDead())
	{
		return;
	}

	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked))
	{
		return;
	}

	if (bIsHitReactMontagePlaying && !bCanMoveDuringHitReact)
	{
		return;
	}

	if (bIsDodgeMontagePlaying)
	{
		return;
	}

	if (bIsReviveMontagePlaying)
	{
		return;
	}

	const FRotator ControlRotation = GetController()->GetControlRotation();
	const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, Forward);
	AddMovementInput(RightDirection, Right);
}

void ABlackoutPlayerCharacter::DoLook(float Yaw, float Pitch)
{
	if (IsDead())
	{
		return;
	}

	/*if (bIsHitReactMontagePlaying)
	{
		return;
	}*/

	if (GetController() != nullptr)
	{
		const UBlackoutGraphicsUserSettings* GraphicsUserSettings = Cast<UBlackoutGraphicsUserSettings>(
			UGameUserSettings::GetGameUserSettings());
		const float AppliedMouseSensitivity = GraphicsUserSettings
			? GraphicsUserSettings->GetMouseSensitivity()
			: 1.0f;

		AddControllerYawInput(Yaw * AppliedMouseSensitivity);
		AddControllerPitchInput(Pitch * AppliedMouseSensitivity);
	}
}

void ABlackoutPlayerCharacter::ToggleFlashlight()
{
	if (IsDead() || IsDowned())
	{
		return;
	}

	bIsFlashlightOn = !bIsFlashlightOn;

	UE_LOG(LogTemp, Warning, TEXT("[Flashlight] ToggleFlashlight Called. LocalState: %s"), bIsFlashlightOn ? TEXT("ON") : TEXT("OFF"));

	// 로컬 빠른 체감 레이턴시 0ms 반응을 위한 선제 갱신
	if (FlashlightComponent)
	{
		FlashlightComponent->SetVisibility(bIsFlashlightOn);
	}

	// 서버로 동기화 요청
	Server_SetFlashlightState(bIsFlashlightOn);
}

void ABlackoutPlayerCharacter::Server_SetFlashlightState_Implementation(bool bNewState)
{
	bIsFlashlightOn = bNewState;
	
	UE_LOG(LogTemp, Warning, TEXT("[Flashlight] Server_SetFlashlightState RPC. ServerState: %s"), bIsFlashlightOn ? TEXT("ON") : TEXT("OFF"));

	if (FlashlightComponent)
	{
		FlashlightComponent->SetVisibility(bIsFlashlightOn);
	}
}

void ABlackoutPlayerCharacter::OnRep_FlashlightOn()
{
	UE_LOG(LogTemp, Warning, TEXT("[Flashlight] OnRep_FlashlightOn Called. ReplicatedState: %s"), bIsFlashlightOn ? TEXT("ON") : TEXT("OFF"));

	if (FlashlightComponent)
	{
		FlashlightComponent->SetVisibility(bIsFlashlightOn);
	}
}

#pragma endregion 
