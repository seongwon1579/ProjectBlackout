#include "Animation/BlackoutEnemyAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "BlackoutGameplayTags.h"
#include "Characters/BlackoutEnemyCharacter.h"

void UBlackoutEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	EnemyCharacter = Cast<ABlackoutEnemyCharacter>(OwnerCharacter);
}

void UBlackoutEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!EnemyCharacter)
	{
		EnemyCharacter = Cast<ABlackoutEnemyCharacter>(OwnerCharacter);
		if (!EnemyCharacter) return;
	}

	// GAS 태그 상태 업데이트
	if (UAbilitySystemComponent* ASC = EnemyCharacter->GetAbilitySystemComponent())
	{
		bIsAttacking = ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Attacking);
	}
}
