// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/BORavagerData.h"

#include "BlackoutPullable.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI, NotBlueprintable)
class UBlackoutPullable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTBLACKOUT_API IBlackoutPullable
{
	GENERATED_BODY()

public:
	virtual void ApplyPull(const FPullData& PullData) = 0;
}; 
