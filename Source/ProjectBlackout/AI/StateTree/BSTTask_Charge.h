#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_Charge.generated.h"

class ABlackoutEnemyCharacter;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_ChargeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABlackoutEnemyCharacter> OwnerCharacter;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float ChargeDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float StaggerImpulse = 500.0f;
};

USTRUCT(meta = (DisplayName = "Root Hollow Charge", Category = "Blackout|Minion"))
struct PROJECTBLACKOUT_API FBSTTask_Charge : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_ChargeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
