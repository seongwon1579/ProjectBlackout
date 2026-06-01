// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_TeleportBase.h"
#include "BlackoutGA_Shrewd_TeleportByEQS.generated.h"

/**
 * 
 */
class UEnvQuery;
struct FEnvQueryResult;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_TeleportByEQS : public UBlackoutGA_Shrewd_TeleportBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="Blackout")
	TObjectPtr<UEnvQuery> TeleportQuery;

	virtual void StartResolveDestination() override;

	
private:
	void OnEQSFinished(TSharedPtr<FEnvQueryResult> Result);
};
