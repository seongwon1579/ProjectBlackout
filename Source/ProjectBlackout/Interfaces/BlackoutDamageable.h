#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayEffectTypes.h"
#include "BlackoutDamageable.generated.h"

UINTERFACE(MinimalAPI, NotBlueprintable)
class UBlackoutDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 보스 히트박스 부위별 피해 배율 분기를 위한 C++ 전용 인터페이스.
 * ABlackoutBossCharacter의 히트박스 컴포넌트 부착 액터가 구현.
 * TDD §5.2: Body.WeakSpot x1.5 / Body.ArmoredLimb x0.5
 */
class PROJECTBLACKOUT_API IBlackoutDamageable
{
	GENERATED_BODY()

public:
	/**
	 * 피격된 본(Bone) 이름으로 부위 태그를 반환.
	 * GE_Damage의 SetByCaller(HitPartTag) 값 결정에 사용.
	 */
	virtual FGameplayTag GetHitPartTag(FName BoneName) const = 0;

	/**
	 * 히트박스 피격 시 호출. GE Spec을 대상 ASC에 적용.
	 * 서버 Authority에서만 호출되어야 함.
	 */
	virtual void ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName) = 0;
};
