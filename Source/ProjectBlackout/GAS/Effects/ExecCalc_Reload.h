#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Reload.generated.h"

/**
 * 장전 실행 계산기 (TDD v5 §4.1)
 * 탄창과 예비탄을 단일 트랜잭션으로 갱신합니다.
 * InstigatorTags를 통해 주무기/보조무기를 구분합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UExecCalc_Reload : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_Reload();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
