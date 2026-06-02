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
	
	// 유지할 거리 
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DesireRange = 1200.0f;
	
	// 거리 허용 오차 
	UPROPERTY(EditAnywhere , Category="Parameter")
	float RangeTolerance = 150.0f;
	
	// 궤도 선회
	UPROPERTY(EditAnywhere, Category="Parameter|Orbit" , meta=(ClampMin="0.0"))
	float OrbitStrength =0.8f;
	UPROPERTY(EditAnywhere, Category="Parameter|Orbit" , meta=(ClampMin="0.1"))
	float ReverseIntervalMin = 2.5f;
	UPROPERTY(EditAnywhere , Category="Parameter|Orbit" , meta=(ClampMin="0.1"))
	float ReverseIntervalMax = 5.0f;
	
	// 수직 
	UPROPERTY(EditAnywhere , Category="Parameter|Vertical")
	float HeightOffset= 300.0f;
	UPROPERTY(EditAnywhere , Category="Parameter|Vertical")
	float BobAmplitude =80.0f;
	UPROPERTY(EditAnywhere, Category="Parameter|Vertical")
	float BobFrequency = 1.5f;
	UPROPERTY(EditAnywhere, Category="Parameter|Vertical" , meta=(ClampMin="1.0"))
	float VerticalTolerance = 150.0f;
	
	// 가변 변경
	UPROPERTY(EditAnywhere , Category="Parameter|Radius")
	float RadiusVarAmplitude = 200.0f;
	UPROPERTY(EditAnywhere , Category="Parameter|Radius")
	float RadiusVarFrequency = 0.5f;
	
	// 내부 상태
	bool bOrbitRight = true;
	float NextReverseTime =0.0f;
	
	UPROPERTY(EditAnywhere , Category="Parameter|Bank" , meta=(ClampMin="0.0" ,ClampMax = "89.0"))
	float MaxBankAngle = 8.0f;
	UPROPERTY(EditAnywhere , Category="Parameter|Bank" , meta=(ClampMin="0.1"))
	float BankInterpSpeed = 1.5f;
	
	FRotator DefaultMeshRelRot = FRotator::ZeroRotator;
	float CurrentBank = 0.0f;
	
};


USTRUCT(meta=(DisplayName="Fly Kite (Maintain Range)") , Category="Blackout|AI")
struct PROJECTBLACKOUT_API FBSTTask_FlyKite : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FBSTTask_FlyKiteInstanceData;
	virtual const UStruct* GetInstanceDataType() const override {return FInstanceDataType::StaticStruct();}
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	
};
