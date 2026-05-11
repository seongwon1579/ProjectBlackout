#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_MoveTowardTarget.generated.h"

class AAIController;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_MoveTowardTargetInstanceData  
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere ,Category="Context")
	TObjectPtr<AAIController> Controller;
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> TargetPawn;
	
	/** 거리 안에 들어오면 Succeeded */
	UPROPERTY(EditAnywhere , Category="Parameter" , meta=(ClampMin="0.0"))
	float AcceptableDistance = 1500.0f;
	
};

USTRUCT(meta=(DisplayName="Move Toward Target" , Category="Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_MoveTowardTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBSTTask_MoveTowardTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};
