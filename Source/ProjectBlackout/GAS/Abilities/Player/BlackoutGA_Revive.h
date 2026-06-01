#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "BlackoutGA_Revive.generated.h"

class ABlackoutPlayerCharacter;
class UAnimMontage;
class UAbilitySystemComponent;

/**
 * 다운된 아군 근처에서 입력 유지 시 부활을 완료하는 플레이어 상호작용 GA.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Revive : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Revive();

	static const UBlackoutGA_Revive* GetActiveReviveAbilityFromActor(const AActor* AvatarActor);
	static float GetReviveRangeForActor(const AActor* AvatarActor);

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	UFUNCTION(BlueprintPure, Category = "Blackout|Ability")
	float GetReviveProgressNormalized() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Ability")
	ABlackoutPlayerCharacter* GetReviveTarget() const { return CachedTarget.Get(); }

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float ReviveDuration = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float ReviveTickInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float ReviveRange = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability", meta = (ClampMin = 0.01, ClampMax = 1.0))
	float RevivedHealthPercent = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	TObjectPtr<UAnimMontage> RevivePerformMontage;

	/** 부활 완료 순간 대상 위치에서 실행할 일회성 GCN 태그입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability|Cue")
	FGameplayTag ReviveCueTag;
	
private:
	UFUNCTION()
	void HandleReviveTick();

	void FinishRevive();
	void CancelRevive();
	void ExecuteReviveCue(UAbilitySystemComponent* TargetAbilitySystemComponent, float RevivedHealth) const;
	ABlackoutPlayerCharacter* FindReviveTarget() const;
	bool CanReviveTarget(const ABlackoutPlayerCharacter* Reviver, const ABlackoutPlayerCharacter* Target) const;

	UPROPERTY(Transient)
	TObjectPtr<ABlackoutPlayerCharacter> CachedReviver;

	UPROPERTY(Transient)
	TObjectPtr<ABlackoutPlayerCharacter> CachedTarget;

	FTimerHandle ReviveTickTimerHandle;
	float ReviveElapsedTime = 0.0f;
	float LocalReviveStartTimeSeconds = 0.0f;
};
