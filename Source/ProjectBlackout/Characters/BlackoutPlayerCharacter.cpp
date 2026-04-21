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

	// TPS: 컨트롤러 회전은 카메라에만 적용, 캐릭터는 이동 방향으로 자동 회전
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ABlackoutPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	BindASCInput();
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
