#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_Teleport.generated.h"

class ABlackoutEnemyCharacter;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_TeleportInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABlackoutEnemyCharacter> OwnerCharacter;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DesiredOffsetRange = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bRequireOutOfSight = true;
};

USTRUCT(meta = (DisplayName = "Teleport", Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_Teleport : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_TeleportInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
