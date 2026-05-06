#pragma once

#include  "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_StrafeAroundTarget.generated.h"

class AAIController;

USTRUCT()
struct PROJECTBLACKOUT_API  FBSTTask_StrafeAroundTargetInstanceData 
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere , Category="Context")
	TObjectPtr<AAIController> Controller;
	
	UPROPERTY(EditAnywhere, Category="Context")
	TObjectPtr<APawn> TargetPawn;
	
	/** 좌 우 전환 주기 */
	UPROPERTY(EditAnywhere , Category="Parameter", meta=(ClampMin=0.1f))
	float StrafeInterval = 1.5f;
	
	/** 전환 시 타겟 기준 회전각도 */
	UPROPERTY(EditAnywhere,Category="Parameter",meta=(ClampMin=1.0f , ClampMax=90.0f))
	float StrafeAngleDeg = 30.0f;
	
	bool bStrafeRight = true;
	float NextSwitchTime = 0.0f;
	float CachedRadius = 0.0f;
};


USTRUCT(meta =(DisplayName="Strafe Around Target" , Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API  FBSTTask_StrafeAroundTarget :public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBSTTask_StrafeAroundTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};
