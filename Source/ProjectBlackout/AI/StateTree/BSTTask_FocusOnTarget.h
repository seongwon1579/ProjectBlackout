#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_FocusOnTarget.generated.h"

class APawn;
class AAIController;

USTRUCT()
struct PROJECTBLACKOUT_API  FBSTTask_FocusOnTargetInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere , Category="Context")
	TObjectPtr<AAIController> Controller;
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> TargetPawn;
};


USTRUCT(meta=(DisplayName="Focus On Target" , Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_FocusOnTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBSTTask_FocusOnTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};
