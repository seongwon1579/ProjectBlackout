#include "Animation/BlackoutPlayerAnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

void UBlackoutPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<ABlackoutPlayerCharacter>(OwnerCharacter);
}

void UBlackoutPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<ABlackoutPlayerCharacter>(OwnerCharacter);
		if (!PlayerCharacter) return;
	}

	// GAS 태그 또는 컴포넌트로부터 상태 정보 업데이트
	// 예: 조준 중인지 확인 (Combat 에픽에서 추가될 태그 등 활용 예정)
	// if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	// {
	//     bIsAiming = ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Aiming")));
	// }
}
