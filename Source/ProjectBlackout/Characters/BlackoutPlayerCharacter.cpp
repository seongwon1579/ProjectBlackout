#include "BlackoutPlayerCharacter.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Components/BlackoutImpactIndicatorComponent.h"
#include "Data/BOCharacterData.h"
#include "Framework/BlackoutPlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlackoutLog.h"
#include "EnhancedInputComponent.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "VerseVM/VVMRuntimeError.h"

ABlackoutPlayerCharacter::ABlackoutPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// TPS 카메라 셋업
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 350.f;
	SpringArm->bUsePawnControlRotation = true;

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
}

void ABlackoutPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CacheAimDefaults();
	UpdateAimMovementMode();
}

void ABlackoutPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlackoutPlayerCharacter, ReplicatedAimOffset, COND_SkipOwner);
}

void ABlackoutPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// aim 모드  틱 로컬 플레이어 카메라 갱신 
	if (!IsLocallyControlled())
	{
		return;
	}

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

			// 클라이언트에서도 어트리뷰트 초기화
			InitializeAttributes();
		}
	}
}

void ABlackoutPlayerCharacter::Server_SetPendingDodgeInput_Implementation(FVector2D NewInput)
{
	PendingDodgeInput = NewInput;
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

// Multicast_PlayDodgeMontage_Implementation 은 TDD §4.1 v2 에서 폐기되었습니다.
// 회피 몽타주 재생은 GAS 표준 PlayMontageAndWait → ASC::PlayMontage 경로로 처리됩니다.

void ABlackoutPlayerCharacter::Multicast_PlayHitReactMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayHitReactMontage(Montage, PlayRate);
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
	BO_LOG_GAS(Log,
		"PlayFireMontage result=%.2f Local=%s Authority=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

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

bool ABlackoutPlayerCharacter::PlayHitReactMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayHitReactMontage failed: Montage가 비어 있음");
		bIsHitReactMontagePlaying = false;
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayHitReactMontage failed: MeshComponent가 비어 있음");
		bIsHitReactMontagePlaying = false;
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayHitReactMontage failed: AnimInstance가 비어 있음");
		bIsHitReactMontagePlaying = false;
		return false;
	}

	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		bIsHitReactMontagePlaying = true;
		BO_LOG_GAS(Verbose, "PlayHitReactMontage skipped: 이미 같은 히트 몽타주가 재생 중임");
		return true;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	if (PlayResult > 0.f)
	{
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ABlackoutPlayerCharacter::HandleHitReactMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
		bIsHitReactMontagePlaying = true;
	}
	else
	{
		bIsHitReactMontagePlaying = false;
	}

	BO_LOG_GAS(Log,
		"PlayHitReactMontage result=%.2f Local=%s Authority=%s Montage=%s",
		PlayResult,
		IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Montage));

	return PlayResult > 0.f;
}

void ABlackoutPlayerCharacter::Multicast_PlayWeaponSwapMontage_Implementation(FGameplayTag TargetWeaponSlotTag, float PlayRate)
{
	PlayWeaponSwapMontage(TargetWeaponSlotTag, PlayRate);
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

void ABlackoutPlayerCharacter::Multicast_PlayConsumableMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
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
	BO_LOG_GAS(Log,
		"Hit react montage ended: Interrupted=%s Montage=%s",
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

void ABlackoutPlayerCharacter::OnHitReact()
{
	Super::OnHitReact();

	if (!HasAuthority())
	{
		return;
	}

	if (!HitReactMontage)
	{
		BO_LOG_GAS(Verbose, "OnHitReact skipped: HitReactMontage가 비어 있음");
		return;
	}

	Multicast_PlayHitReactMontage(HitReactMontage, 1.f);
}

void ABlackoutPlayerCharacter::HandleDownedStateChanged()
{
	const bool bWasDowned = bWasDownedLocally;
	const bool bIsCurrentlyDowned = IsDowned();

	bWasDownedLocally = bIsCurrentlyDowned;

	if (bIsCurrentlyDowned)
	{
		ApplyDownedStateLocally();
		return;
	}

	ClearDownedStateLocally();

	if (bWasDowned && !IsDead() && ReviveMontage)
	{
		PlayReviveMontage(ReviveMontage, 1.f);
	}
}

void ABlackoutPlayerCharacter::ApplyDownedStateLocally()
{
	bIsHitReactMontagePlaying = false;
	bIsDodgeMontagePlaying = false;
	bIsWeaponSwapMontagePlaying = false;

	if (CombatComponent)
	{
		CombatComponent->StopFire();
		CombatComponent->HandlePrimaryActionReleased();
		CombatComponent->StopAim();
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

	ApplyDownedStateLocally();

	if (DownedEnterMontage)
	{
		Multicast_PlayDownedEnterMontage(DownedEnterMontage, 1.f);
	}
}

bool ABlackoutPlayerCharacter::CanEnterDownedState() const
{
	return !IsDead() && !IsDowned();
}

void ABlackoutPlayerCharacter::OnDeath()
{
	if (IsDead())
	{
		return;
	}

	Super::OnDeath();

	bIsHitReactMontagePlaying = false;
	bIsDodgeMontagePlaying = false;
	bIsWeaponSwapMontagePlaying = false;

	if (CombatComponent)
	{
		CombatComponent->StopFire();
		CombatComponent->HandlePrimaryActionReleased();
		CombatComponent->StopAim();
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	if (DeathMontage)
	{
		Multicast_PlayDeathMontage(DeathMontage, 1.f);
	}
}

void ABlackoutPlayerCharacter::Server_ReviveFromDowned_Implementation(float RevivedHealth)
{
	if (!HasAuthority() || !AbilitySystemComponent || !IsDowned() || IsDead())
	{
		return;
	}

	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	const float ClampedHealth = FMath::Clamp(RevivedHealth, 1.f, MaxHealth);

	AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Downed);
	bIsDowned = false;
	AbilitySystemComponent->SetNumericAttributeBase(UBlackoutBaseAttributeSet::GetHealthAttribute(), ClampedHealth);
	ClearDownedStateLocally();
	BO_LOG_GAS(Log, "ReviveFromDowned: Target=%s Health=%.1f", *GetName(), ClampedHealth);
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

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
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
	const float TargetFOV = bIsAiming ? AimFOV : DefaultFOV;

	
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

	if (bIsHitReactMontagePlaying)
	{
		return;
	}

	if (bIsDodgeMontagePlaying)
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
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

#pragma endregion 
