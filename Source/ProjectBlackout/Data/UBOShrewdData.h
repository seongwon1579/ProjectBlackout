// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UBOShrewdData.generated.h"

class UGameplayAbility;
/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UUBOShrewdData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MaxHealth = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MovementSpeed = 400.f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Blackout|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
};
