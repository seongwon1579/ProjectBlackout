#include "Animation/BlackoutBossAnimInstance.h"
#include "Characters/BlackoutBossCharacter.h"

void UBlackoutBossAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BossCharacter = Cast<ABlackoutBossCharacter>(OwnerCharacter);
}

void UBlackoutBossAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!BossCharacter)
	{
		BossCharacter = Cast<ABlackoutBossCharacter>(OwnerCharacter);
		if (!BossCharacter) return;
	}

	// 보스 전용 데이터 업데이트 로직 (페이즈, 그로기 상태 등)
	// 예: bIsGroggy = BossCharacter->GetIsGroggy();
}
