#pragma once

#include "CoreMinimal.h"
#include "Core/BlackoutTypes.h"
#include "GameplayTagContainer.h"
#include "BlackoutMatchResultTypes.generated.h"

class ABlackoutPlayerState;

UENUM(BlueprintType)
enum class EBlackoutMatchResultStatFormat : uint8
{
	Integer UMETA(DisplayName = "Integer"),
	Percent UMETA(DisplayName = "Percent"),
};

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutMatchResultSummaryData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	bool bIsVictory = true;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	EBossType DefeatedBossType = EBossType::Main;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	FText ResultTitle = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	FText ResultSubtitle = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 TotalDamageDealt = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 TotalKills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 TotalRevives = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	float TeamAccuracyPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	float AutoTravelRemainingTime = 0.0f;
};

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutMatchResultPlayerStatsData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	TObjectPtr<ABlackoutPlayerState> PlayerState = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	FText DisplayName = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	FGameplayTag SelectedClassTag;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 DisplayOrder = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 DamageDealt = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 MeleeKills = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 ShotsFired = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 ShotsHit = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	float AccuracyPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 ConsumablesUsed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	int32 Revives = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	bool bIsLocalPlayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Result")
	bool bIsValid = false;
};

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutMatchResultStatRowDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Result")
	FName StatId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Result")
	FText DisplayLabel = FText::GetEmpty();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Result")
	EBlackoutMatchResultStatFormat DisplayFormat = EBlackoutMatchResultStatFormat::Integer;
};
