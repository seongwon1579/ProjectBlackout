#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BlackoutPartyTypes.generated.h"

class ABlackoutPlayerState;

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutPartyMemberStatusData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	TObjectPtr<ABlackoutPlayerState> PlayerState = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FText DisplayName = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FGameplayTag SelectedClassTag;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float CurrentHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float MaxHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float NormalizedHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsDowned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsReviveInteractionActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsLocalPlayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsValid = false;
};
