#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_RetreatFromTarget.generated.h"

class AAIController;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_RetreatFromTargetInstanceData
{
	GENERATED_BODY()
	
	
	UPROPERTY(EditAnywhere , Category="Context")
	TObjectPtr<AAIController> Controller;
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> TargetPawn;
	
	/** (2D ,cm ) 거리 이상 멀어지면 Succeeded */
	UPROPERTY(EditAnywhere,Category="Parameter" ,meta=(ClampMin="0.0"))
	float SafeDistance =1100.0f;
	
	/** 여유 후퇴 거리 */
	UPROPERTY(EditAnywhere, Category="Parameter" ,meta=(ClampMin="0.0"))
	float OvershootBuffer =200.0f;
};

USTRUCT(meta=(DisplayName="Retreat From Target", Category="Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_RetreatFromTarget:public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBSTTask_RetreatFromTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
