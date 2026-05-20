#pragma once

#include "CoreMinimal.h"
#include "Data/BORavagerData.h"
#include "GAS/Abilities/BlackoutEnemyGameplayAbility.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutBossGameplayAbility.generated.h"

class ABlackoutBossCharacter;
class UAnimMontage;

UCLASS(Abstract)
class PROJECTBLACKOUT_API UBlackoutBossGameplayAbility : public UBlackoutEnemyGameplayAbility
{
	GENERATED_BODY()
public:
	UBlackoutBossGameplayAbility();
	
protected:
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {}
	virtual void PostActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {}
	virtual void SetupEventListeners() {}
	virtual FGameplayTag SelectMontageTag(const FGameplayEventData* TriggerEventData) const;
	virtual bool IsValid() const;
	
	bool TryResolveMontage(const FGameplayEventData* TriggerEventData);
	void TrySetupMotionWarp(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	void PlayMontage();
	UAnimMontage* GetMontage(const FGameplayTag& Tag);
	
	UFUNCTION()
	virtual void OnMontageEnded();
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data", meta = (Categories = "Ability"))
	FGameplayTag PatternDataTag;
	
	UPROPERTY(Transient)
	TObjectPtr<UBORavagerData> CachedPatternData;
	
	UPROPERTY(Transient)
	TObjectPtr<ABlackoutBossCharacter> CachedOwner;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> SelectedMontage;
	
	static const FName WarpTargetName;
};
