// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_SpawnProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_SpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	UPROPERTY(EditAnywhere, Category = "Event", meta = (Categories = "Event"))
	FGameplayTag EventTag;
};
