// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutBossAnimInstance.h"
#include "BlackoutRavagerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutRavagerAnimInstance : public UBlackoutBossAnimInstance
{
	GENERATED_BODY()

protected:
	virtual void UpdateAnimationProperties() override;

	/** 지면 이동 속도 (블렌드 스페이스 Speed 축) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Speed;

	/** 이동 방향 (-180 ~ 180, 블렌드 스페이스 Direction 축) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float Direction;

	/** 이동 방향 (-180 ~ 180, 블렌드 스페이스 Direction 축) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation|BlendSpace")
	float AimDirection;
	
};
