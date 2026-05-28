#include "Animation/BlackoutBossAnimInstance.h"

void UBlackoutBossAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	UpdateAnimationProperties();
}