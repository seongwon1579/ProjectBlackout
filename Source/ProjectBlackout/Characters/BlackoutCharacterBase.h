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
	ABlackoutCharacterBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual FGameplayTag GetHitPartTag(FName BoneName) const override;
	virtual void ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	bool IsDead() const { return bIsDead; }
	bool IsDowned() const { return bIsDowned; }

protected:
	virtual void BeginPlay() override;

	/** 공용 죽음 상태 중복죽음 또는 죽음 후 추가 입력등을 막기용  */
	UPROPERTY()
	bool bIsDead = false;

	/** 플레이어 전용 다운 상태 여부. 향후 부활 상호작용의 기준으로 사용합니다. */
	UPROPERTY(ReplicatedUsing = OnRep_DownedState)
	bool bIsDowned = false;
	
	/** 공통 데미지 GE Spec 적용 경로. 무적 태그와 피격 반응도 여기서 함께 처리합니다. */
	bool ApplyIncomingDamageSpec(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName);

	/** 서브클래스에서 초기화 후 설정. 직접 소유하지 않으므로 UPROPERTY로 참조만 유지. */
	UPROPERTY()
	TObjectPtr<UBlackoutAbilitySystemComponent> AbilitySystemComponent;

	/** Health가 0이 됐을 때 AttributeSet → 캐릭터로 전달되는 사망 처리 진입점. */
	virtual void OnDeath();

	/** Health가 0이 되었을 때 다운 상태를 지원하는 캐릭터가 진입하는 공통 훅입니다. */
	virtual void OnDowned();

	/** HP 0 도달 시 즉시 사망 대신 다운 상태로 전환할 수 있는지 여부입니다. */
	virtual bool CanEnterDownedState() const;

	/** 피격 시 히트 리액션 몽타주 재생 등 공통 처리. */
	virtual void OnHitReact();

	/** 기절(State.Stun) 태그 부여 시 이동/액션 봉쇄 처리. */
	virtual void OnStun();

	/** 다운 상태 복제 시 클라이언트에서 로컬 반응을 맞추기 위한 진입점입니다. */
	UFUNCTION()
	void OnRep_DownedState();

	/** 다운 상태 변경 시 서브클래스가 로컬 전용 후처리를 구현할 수 있는 훅입니다. */
	virtual void HandleDownedStateChanged();
};
