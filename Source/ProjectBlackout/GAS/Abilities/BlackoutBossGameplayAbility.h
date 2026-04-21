#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutBossGameplayAbility.generated.h"

/**
 * 보스 패턴 GA 전용 베이스 클래스.
 * 모든 보스 GA(Shrewd / Ravager)는 이 클래스를 상속한다.
 *
 * - NetExecutionPolicy = ServerOnly : 보스 AI는 서버 단독 구동이므로
 *   클라이언트 예측이 불필요하다. 연출(애니메이션, GCN, GE)만 리플리케이션.
 * - InstancingPolicy = InstancedPerActor : 페이즈별로 여러 패턴이 순차 발동되므로
 *   GA 인스턴스별 상태 관리가 필요하다.
 * - 향후 공통 정책(패턴 중첩 방지 ActivationBlockedTags, 쿨다운 태그 등)은
 *   이 클래스 한 곳에서 추가한다.
 */
UCLASS(Abstract)
class PROJECTBLACKOUT_API UBlackoutBossGameplayAbility : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutBossGameplayAbility();
};
