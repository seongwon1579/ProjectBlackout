#include "BlackoutGameplayAbility.h"
#include "GameplayTags/BlackoutGameplayTags.h"

UBlackoutGameplayAbility::UBlackoutGameplayAbility()
{
	// 기본값: 서버에서 예측(Predict)하고 클라이언트에 동기화
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 완전 사망 상태에서는 플레이어/AI 공통으로 새 어빌리티 시작을 막습니다.
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Dead);
}
