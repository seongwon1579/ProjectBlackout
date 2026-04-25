#include "GAS/Effects/ExecCalc_CombatReward.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"

UExecCalc_CombatReward::UExecCalc_CombatReward()
{
}

void UExecCalc_CombatReward::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// TODO: Source PlayerState의 SelectedClassTag와 Kill 태그 조합 검증 로직 구현
	// 예: Kill.Melee, Kill.MultiTarget, Kill.WeakSpot 검사 후 보상(BloodRoot/GulSerum 등) 지급
}
