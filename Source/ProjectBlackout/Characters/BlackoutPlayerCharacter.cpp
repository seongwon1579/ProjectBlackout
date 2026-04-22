#include "BlackoutPlayerCharacter.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Data/BOCharacterData.h"
#include "AbilitySystemInterface.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/PlayerState.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlackoutLog.h"
#include "EnhancedInputComponent.h"

ABlackoutPlayerCharacter::ABlackoutPlayerCharacter()
{
	// TPS 카메라 셋업
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 350.f;
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// TPS: 컨트롤러 회전은 카메라에만 적용, 캐릭터는 이동 방향으로 자동 회전 x
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void ABlackoutPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	BindASCInput();
	
	
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABlackoutPlayerCharacter::Move);
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
				BO_LOG_GAS(Log, "Abilities granted to %s", *GetName());
			}

			// 입력 바인딩 시도
			BindASCInput();
		}
	}
}

void ABlackoutPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// Client: InitAbilityActorInfo
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetPlayerState()))
	{
		AbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(
			ASCInterface->GetAbilitySystemComponent());

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);

			// 클라이언트에서도 어트리뷰트 초기화
			InitializeAttributes();

			// 입력 바인딩 시도
			BindASCInput();
		}
	}
}

void ABlackoutPlayerCharacter::BindASCInput()
{
	if (!bIsInputBound && AbilitySystemComponent && InputComponent)
	{
		// GAS의 입력 매핑 정보를 정의
		// EBlackoutAbilityInputID 열거형과 문자열 이름을 연결
		FTopLevelAssetPath EnumAssetPath = FTopLevelAssetPath(StaticEnum<EBlackoutAbilityInputID>());
		
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent,
			FGameplayAbilityInputBinds(
				FString("Confirm"),
				FString("Cancel"),
				EnumAssetPath,
				static_cast<int32>(EBlackoutAbilityInputID::Confirm),
				static_cast<int32>(EBlackoutAbilityInputID::Cancel)
			)
		);

		bIsInputBound = true;
		BO_LOG_GAS(Log, "ASC input bound for %s", *GetName());
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
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_MaxStamina, CharacterData->BaseMaxStamina);
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_MovementSpeed, CharacterData->BaseMovementSpeed);

		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		BO_LOG_GAS(Log, "Attributes initialized for %s using GE", *GetName());
	}
}


#pragma region InputSetup

void ABlackoutPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
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

	if (Forward > 0.f)
	{
		const FRotator TargetRotation(0.f, GetController()->GetControlRotation().Yaw, 0.f);
		const FRotator NewRotation = FMath::RInterpTo(
			GetActorRotation(),
			TargetRotation,
			GetWorld()->GetDeltaSeconds(),
			ForwardTurnInterpSpeed);

		SetActorRotation(NewRotation);
	}

	AddMovementInput(GetActorForwardVector(), Forward);
	AddMovementInput(GetActorRightVector(), Right);
	
	
	
	/*if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}*/
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
