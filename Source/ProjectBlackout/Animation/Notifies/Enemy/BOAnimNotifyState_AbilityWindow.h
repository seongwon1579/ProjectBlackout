// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "BOAnimNotifyState_AbilityWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotifyState_AbilityWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, Category = "Event", meta = (Categories = "Event"))
	FGameplayTag EventStart;
	
	UPROPERTY(EditAnywhere, Category = "Event", meta = (Categories = "Event"))
	FGameplayTag EventEnd;
};
