#include "BlackoutGameplayAbility.h"

UBlackoutGameplayAbility::UBlackoutGameplayAbility()
{
	// 기본값: 서버에서 예측(Predict)하고 클라이언트에 동기화
	ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}
