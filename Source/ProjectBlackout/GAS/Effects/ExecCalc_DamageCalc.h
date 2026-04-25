#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_DamageCalc.generated.h"

/**
 * GE_Damage 전용 실행 계산기.
 * SetByCaller(Data.Damage) × 부위 배율(Body.*) × (1 - DamageReduction) = 최종 피해.
 * TDD §5.2: Body.WeakSpot x1.5 / Body.ArmoredLimb x0.5
 */
UCLASS()
class PROJECTBLACKOUT_API UExecCalc_DamageCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_DamageCalc();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
