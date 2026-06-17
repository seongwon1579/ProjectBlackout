// ─── 구현 내역 ───────────────────────
//  - 최승현: 타겟까지의 거리가 Min/Max 사거리(2D·3D 선택) 안인지 판정하는 StateTree 조건
// ──────────────────────────────────────

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
	UPROPERTY(EditAnywhere,Category="Parameter" , meta=(ClampMin="0.0"))
	float MinRange = 0.0f;
	
	/** 사거리 상한 (포함) */
	UPROPERTY(EditAnywhere , Category="Parameter" , meta=(ClampMin="0.0"))
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
