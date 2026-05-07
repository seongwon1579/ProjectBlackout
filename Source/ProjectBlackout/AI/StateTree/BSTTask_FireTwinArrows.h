#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_FireTwinArrows.generated.h"

class APawn;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_FireTwinArrowsInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> OwnerPawn;
};

USTRUCT(meta = (DisplayName = "Fire Twin Arrows", Category = "Blackout|Minion"))
struct PROJECTBLACKOUT_API FBSTTask_FireTwinArrows : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_FireTwinArrowsInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
