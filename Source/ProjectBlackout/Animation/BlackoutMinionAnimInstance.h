// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutEnemyAnimInstance.h"
#include "BlackoutMinionAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutMinionAnimInstance : public UBlackoutEnemyAnimInstance
{
	GENERATED_BODY()

public:
	void OnDeath();

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Direction;

	// UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	// float bIsDead = false;
};
