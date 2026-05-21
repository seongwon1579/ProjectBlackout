// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackoutInteractable.h"
#include "GameFramework/Actor.h"
#include "BlackoutCheckpoint.generated.h"

UCLASS()
class PROJECTBLACKOUT_API ABlackoutCheckpoint : public AActor, public IBlackoutInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABlackoutCheckpoint();
	
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category = "Blackout|Checkpoint")
	FText InteractionPrompt;
};
