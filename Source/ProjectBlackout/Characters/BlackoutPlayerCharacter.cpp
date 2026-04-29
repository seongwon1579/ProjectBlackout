#include "BlackoutPlayerCharacter.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Data/BOCharacterData.h"
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
#include "Net/UnrealNetwork.h"

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
				AbilitySystemComponent->GiveDefaultAbilities(CharacterData->GrantedAbilities);

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

void ABlackoutPlayerCharacter::Server_SetAimOffset_Implementation(FVector2D NewAimOffset)
{
	ReplicatedAimOffset = FVector2D(
		FMath::Clamp(NewAimOffset.X, -180.f, 180.f),
		FMath::Clamp(NewAimOffset.Y, -90.f, 90.f));
}

void ABlackoutPlayerCharacter::Multicast_PlayDodgeMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	PlayDodgeMontage(Montage, PlayRate);
}

bool ABlackoutPlayerCharacter::PlayDodgeMontage(UAnimMontage* Montage, float PlayRate)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayDodgeMontage failed: Montage가 비어 있음");
		bIsDodgeMontagePlaying = false;
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayDodgeMontage failed: MeshComponent가 비어 있음");
		bIsDodgeMontagePlaying = false;
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayDodgeMontage failed: AnimInstance가 비어 있음");
		bIsDodgeMontagePlaying = false;
		return false;
	}

	// 로컬 예측 재생 후 멀티캐스트가 도착해도 같은 몽타주를 다시 시작하지 않도록 방지
	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		bIsDodgeMontagePlaying = true;
		BO_LOG_GAS(Verbose, "PlayDodgeMontage skipped: 이미 같은 몽타주가 재생 중임");
		return true;
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	if (PlayResult > 0.f)
	{
		FOnMontageEnded MontageEndedDelegate;
		MontageEndedDelegate.BindUObject(this, &ABlackoutPlayerCharacter::HandleDodgeMontageEnded);
		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, Montage);
		bIsDodgeMontagePlaying = true;
	}
	else
	{
		bIsDodgeMontagePlaying = false;
	}

	BO_LOG_GAS(Log,
		"PlayDodgeMontage result=%.2f Local=%s Authority=%s Montage=%s",
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
	UAnimMontage* Montage = GetWeaponSwapMontage(TargetWeaponSlotTag);
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

void ABlackoutPlayerCharacter::Multicast_PlayMeleeMontage_Implementation(UAnimMontage* Montage, FName StartSection, float PlayRate)
{
	PlayMeleeMontage(Montage, StartSection, PlayRate);
}

bool ABlackoutPlayerCharacter::PlayMeleeMontage(UAnimMontage* Montage, FName StartSection, float PlayRate)
{
	if (!Montage)
	{
		BO_LOG_GAS(Warning, "PlayMeleeMontage failed: Montage가 비어 있음");
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		BO_LOG_GAS(Warning, "PlayMeleeMontage failed: MeshComponent가 비어 있음");
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		BO_LOG_GAS(Warning, "PlayMeleeMontage failed: AnimInstance가 비어 있음");
		return false;
	}

	if (AnimInstance->GetCurrentActiveMontage() == Montage && AnimInstance->Montage_IsPlaying(Montage))
	{
		const FName CurrentSectionName = AnimInstance->Montage_GetCurrentSection(Montage);
		if (StartSection == NAME_None || CurrentSectionName == StartSection)
		{
			BO_LOG_GAS(Verbose, "PlayMeleeMontage skipped: 이미 같은 근접 몽타주/섹션이 재생 중임");
			return true;
		}
	}

	const float PlayResult = PlayAnimMontage(Montage, PlayRate);
	if (PlayResult <= 0.f)
	{
		BO_LOG_GAS(Warning, "PlayMeleeMontage failed: Montage=%s", *GetNameSafe(Montage));
		return false;
	}

	if (StartSection != NAME_None)
	{
		if (Montage->GetSectionIndex(StartSection) == INDEX_NONE)
		{
			BO_LOG_GAS(Warning,
				"PlayMeleeMontage failed: StartSection=%s 이(가) Montage=%s 에 없음",
				*StartSection.ToString(),
				*GetNameSafe(Montage));
			return false;
		}

		AnimInstance->Montage_JumpToSection(StartSection, Montage);
	}

	return true;
}

void ABlackoutPlayerCharacter::Multicast_JumpMeleeMontageSection_Implementation(UAnimMontage* Montage, FName SectionName)
{
	JumpMeleeMontageSection(Montage, SectionName);
}

bool ABlackoutPlayerCharacter::JumpMeleeMontageSection(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage || SectionName == NAME_None)
	{
		return false;
	}

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent)
	{
		return false;
	}

	UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	if (AnimInstance->GetCurrentActiveMontage() != Montage || !AnimInstance->Montage_IsPlaying(Montage))
	{
		return false;
	}

	if (Montage->GetSectionIndex(SectionName) == INDEX_NONE)
	{
		BO_LOG_GAS(Warning,
			"JumpMeleeMontageSection failed: Section=%s 이(가) Montage=%s 에 없음",
			*SectionName.ToString(),
			*GetNameSafe(Montage));
		return false;
	}

	if (AnimInstance->Montage_GetCurrentSection(Montage) == SectionName)
	{
		return true;
	}

	AnimInstance->Montage_JumpToSection(SectionName, Montage);
	return AnimInstance->Montage_GetCurrentSection(Montage) == SectionName;
}

void ABlackoutPlayerCharacter::Multicast_StopMeleeMontage_Implementation(UAnimMontage* Montage, float BlendOutTime)
{
	StopMeleeMontage(Montage, BlendOutTime);
}

bool ABlackoutPlayerCharacter::StopMeleeMontage(UAnimMontage* Montage, float BlendOutTime)
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

void ABlackoutPlayerCharacter::CommitPendingWeaponSwap()
{
	if (CombatComponent)
	{
		CombatComponent->CommitPendingWeaponSwap();
	}
}

void ABlackoutPlayerCharacter::HandleDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsDodgeMontagePlaying = false;
	BO_LOG_GAS(Log,
		"Dodge montage ended: Interrupted=%s Montage=%s",
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

UAnimMontage* ABlackoutPlayerCharacter::GetWeaponSwapMontage(FGameplayTag TargetWeaponSlotTag) const
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

	const bool bIsAiming = CombatComponent->IsAiming();

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

	if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked))
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
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

#pragma endregion 
