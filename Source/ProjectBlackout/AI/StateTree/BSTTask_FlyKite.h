#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_FlyKite.generated.h"

class AAIController;
class APawn;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_FlyKiteInstanceData
{
	GENERATED_BODY()

	// 컨트롤러 (Context 바인딩 -> Pawn 획득)
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> Controller;

	// 어그로 타겟 (Evaluator OutTarget 바인딩)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<APawn> Target;

	// 유지할 궤도 반경
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DesireRange = 1200.0f;

	// 선회 각속도(deg/s) — 반경 * (이 값의 rad) 이 MaxFlySpeed 이하가 되게 튜닝
	UPROPERTY(EditAnywhere, Category = "Parameter|Orbit", meta = (ClampMin = "0.0"))
	float OrbitAngularSpeed = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter|Orbit", meta = (ClampMin = "0.1"))
	float ReverseIntervalMin = 2.5f;
	UPROPERTY(EditAnywhere, Category = "Parameter|Orbit", meta = (ClampMin = "0.1"))
	float ReverseIntervalMax = 5.0f;

	// 수직 (타겟 위 고도 + 부유)
	UPROPERTY(EditAnywhere, Category = "Parameter|Vertical")
	float HeightOffset = 300.0f;
	UPROPERTY(EditAnywhere, Category = "Parameter|Vertical")
	float BobAmplitude = 80.0f;
	UPROPERTY(EditAnywhere, Category = "Parameter|Vertical")
	float BobFrequency = 1.5f;

	// 가변 반경
	UPROPERTY(EditAnywhere, Category = "Parameter|Radius")
	float RadiusVarAmplitude = 200.0f;
	UPROPERTY(EditAnywhere, Category = "Parameter|Radius")
	float RadiusVarFrequency = 0.5f;

	// 내부 상태
	float OrbitAngle = 0.0f;   // 타겟 기준 현재 선회 각(rad)
	bool bOrbitRight = true;
	float NextReverseTime = 0.0f;
};

USTRUCT(meta = (DisplayName = "Fly Kite (Maintain Range)", Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_FlyKite : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_FlyKiteInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
