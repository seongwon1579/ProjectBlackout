// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackoutInteractable.h"
#include "GameFramework/Actor.h"
#include "BlackoutClassSelectStone.generated.h"

class UBoxComponent;

// 캐릭터 변경 상호작용 클래스
UCLASS()
class PROJECTBLACKOUT_API ABlackoutClassSelectStone : public AActor , public IBlackoutInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABlackoutClassSelectStone();
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:
	// Called when the game starts or when spawned
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|ClassSelect")
	TObjectPtr<UBoxComponent> Trigger;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|ClassSelect")
	FText InteractionPrompt;


};
