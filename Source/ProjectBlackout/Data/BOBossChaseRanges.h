#pragma once

#include "CoreMinimal.h"
#include "BOBossChaseRanges.generated.h"

USTRUCT(BlueprintType)           
struct FBossChaseRanges            
{
	GENERATED_BODY()              

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ChaseStartRange = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ChaseEndRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackRange = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackRangeVariance = 0.f;
};