#pragma once

#include "CoreMinimal.h"
#include "Data/BORavagerPatternData.h"
#include "GAS/Abilities/BlackoutEnemyGameplayAbility.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Ravager_Base.generated.h"

class ABlackoutBossCharacter;
class UAnimMontage;

UCLASS(Abstract)
class PROJECTBLACKOUT_API UBlackoutGA_Ravager_Base : public UBlackoutEnemyGameplayAbility
{
	GENERATED_BODY()
public:
	UBlackoutGA_Ravager_Base();
	
protected:
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	virtual void PreActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {}
	virtual void PostActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) {}
	virtual void SetupEventListeners() {}
	virtual FGameplayTag SelectMontageTag(const FGameplayEventData* TriggerEventData) const;
	virtual bool CanActivatePattern() const;
	virtual bool HasValidSettings() const { return true; }  
	
	bool TryResolveMontage(const FGameplayEventData* TriggerEventData);
	USceneComponent* TrySetupMotionWarp(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData);
	void PlayMontage();
	UAnimMontage* GetMontage(const FGameplayTag& Tag);
	
	UFUNCTION()
	virtual void OnMontageEnded();
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Data", meta = (Categories = "Ability"))
	FGameplayTag PatternDataTag;
	
	UPROPERTY(Transient)
	TObjectPtr<UBORavagerPatternData> CachedPatternData;
	
	UPROPERTY(Transient)
	TObjectPtr<ABlackoutBossCharacter> CachedOwner;
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> SelectedMontage;
	
	static const FName WarpTargetName;
};
