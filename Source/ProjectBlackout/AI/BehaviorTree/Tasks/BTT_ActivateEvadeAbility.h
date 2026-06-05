// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/BehaviorTree/Tasks/BTT_ActivateAbility.h"
#include "BTT_ActivateEvadeAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTT_ActivateEvadeAbility : public UBTT_ActivateAbility
{
	GENERATED_BODY()

public:
	UBTT_ActivateEvadeAbility();
	
protected:
	virtual FGameplayTag ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const override;
	virtual FString GetStaticDescription() const override;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability", meta = (Categories = "Ability"))
	FGameplayTag LeftAbilityTag;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability", meta = (Categories = "Ability"))
	FGameplayTag RightAbilityTag;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector EvadeDirectionKey;
};
