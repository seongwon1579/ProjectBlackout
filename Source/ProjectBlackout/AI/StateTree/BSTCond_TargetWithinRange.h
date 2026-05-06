#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "BSTCond_TargetWithinRange.generated.h"


USTRUCT()
struct PROJECTBLACKOUT_API FBstCond_TargetWithinRangeInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere , Category="Combat")
	TObjectPtr<APawn> OwnerPawn;
	
	UPROPERTY(EditAnywhere, Category="Combat")
	TObjectPtr<APawn> TargetPawn;
	
	/**  사거리 하한 , 0이면 하한 없음 */
	UPROPERTY(EditAnywhere,Category="Parameter" , meta=(ClampMin=0.0f))
	float MinRange = 0.0f;
	
	/** 사거리 상한 (포함) */
	UPROPERTY(EditAnywhere , Category="Parmeter" , meta=(ClampMin=0.0))
	float MaxRange = 1500.0f;
	
	/** XY 평면 거리로 비교, false 면 3d */
	UPROPERTY(EditAnywhere,Category="Parameter")
	bool bUse2DDistance = true;
	 
};

USTRUCT()
struct PROJECTBLACKOUT_API  FBSTCond_TargetWithinRange:public FStateTreeConditionCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBstCond_TargetWithinRangeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
	
};
