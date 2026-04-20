#include "BlackoutPlayerCharacter.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Data/BOCharacterData.h"
#include "AbilitySystemInterface.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlackoutLogCategories.h"

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

void ABlackoutPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// PlayerState가 IAbilitySystemInterface를 구현하면 ASC를 가져와 초기화
	// (ABlackoutPlayerState 헤더 직접 의존 없이 인터페이스로 접근)
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(GetPlayerState()))
	{
		AbilitySystemComponent = Cast<UBlackoutAbilitySystemComponent>(
			ASCInterface->GetAbilitySystemComponent());

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);

			if (CharacterData)
			{
				AbilitySystemComponent->GiveDefaultAbilities(CharacterData->GrantedAbilities);
				UE_LOG(LogBlackoutGAS, Log, TEXT("Abilities granted to %s"), *GetName());
			}
		}
	}
}
