#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Interfaces/BlackoutDamageable.h"
#include "BlackoutCharacterBase.generated.h"

class UBlackoutAbilitySystemComponent;

/**
 * 모든 캐릭터(플레이어/적/보스)의 최상위 베이스 클래스.
 * ASC 접근 인터페이스를 일관되게 제공하고, 공통 반응 함수(사망/피격/기절)를 선언.
 *
 * - 플레이어: ABlackoutPlayerState가 ASC를 소유 → PossessedBy에서 포인터 캐시
 * - 적/보스:  자기 자신이 ASC를 소유 → 생성자에서 포인터 캐시
 */
UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutCharacterBase : public ACharacter, public IAbilitySystemInterface, public IBlackoutDamageable
{
	GENERATED_BODY()

public:
	ABlackoutCharacterBase();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual FGameplayTag GetHitPartTag(FName BoneName) const override;
	virtual void ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName) override;

protected:
	virtual void BeginPlay() override;

	/** 서브클래스에서 초기화 후 설정. 직접 소유하지 않으므로 UPROPERTY로 참조만 유지. */
	UPROPERTY()
	TObjectPtr<UBlackoutAbilitySystemComponent> AbilitySystemComponent;

	/** Health가 0이 됐을 때 AttributeSet → 캐릭터로 전달되는 사망 처리 진입점. */
	virtual void OnDeath();

	/** 피격 시 히트 리액션 몽타주 재생 등 공통 처리. */
	virtual void OnHitReact();

	/** 기절(State.Stun) 태그 부여 시 이동/액션 봉쇄 처리. */
	virtual void OnStun();
};
