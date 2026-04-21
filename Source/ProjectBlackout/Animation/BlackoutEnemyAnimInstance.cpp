#include "Animation/BlackoutEnemyAnimInstance.h"
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

	// AI 상태나 태그를 통해 bIsAttacking 등의 상태 업데이트 로직이 들어갈 자리
}
