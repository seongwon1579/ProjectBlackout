// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_Base.h"
#include "BlackoutGA_Shrewd_TeleportBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_TeleportBase : public UBlackoutGA_Shrewd_Base
{
	GENERATED_BODY()
	
protected:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void PrepareAbility() override;
	
	virtual void SetupEventListeners() override;
	
	virtual void StartResolveDestination() {}
	
	UFUNCTION()
	void OnVanishEvent(FGameplayEventData Payload);

	UFUNCTION()
	void OnAppearEvent(FGameplayEventData Payload);

	void RemoveInvulnerableTag();
	void SetTeleportVisualsHidden(bool bHidden);

	FVector CachedDestination = FVector::ZeroVector;
	FRotator CachedTeleportRotation = FRotator::ZeroRotator;

	TMap<TWeakObjectPtr<UPrimitiveComponent>, bool> PreviousTeleportHiddenStates;
};
