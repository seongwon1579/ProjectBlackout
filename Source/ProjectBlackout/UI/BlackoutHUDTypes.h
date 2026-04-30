#pragma once

#include "CoreMinimal.h"
#include "BlackoutHUDTypes.generated.h"

USTRUCT(BlueprintType)
struct FBlackoutImpactIndicatorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bIsVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bHasBlockingHit = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bTargetMismatch = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bUsesProjectilePrediction = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bIsOccludedFromCamera = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector TraceEndLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	float DistanceFromMuzzle = 0.0f;
};
