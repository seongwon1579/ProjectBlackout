#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_CombatReward.generated.h"

/**
 * 전투 보상 실행 계산기 (TDD v5 §5.1)
 * 클래스 및 킬 조건에 따라 자원이나 버프(GE 연쇄)를 적용합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UExecCalc_CombatReward : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_CombatReward();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
