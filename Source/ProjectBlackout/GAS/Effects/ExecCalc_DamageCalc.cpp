#include "ExecCalc_DamageCalc.h"
#include "Attributes/BlackoutBaseAttributeSet.h"
#include "BlackoutGameplayTags.h"
#include "BlackoutLog.h"

namespace
{
	struct FDamageCalcStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(DamageReduction);

		FDamageCalcStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UBlackoutBaseAttributeSet, DamageReduction, Target, false);
		}
	};

	const FDamageCalcStatics& GetStatics()
	{
		static FDamageCalcStatics Statics;
		return Statics;
	}
}

UExecCalc_DamageCalc::UExecCalc_DamageCalc()
{
	RelevantAttributesToCapture.Add(GetStatics().DamageReductionDef);
}

void UExecCalc_DamageCalc::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 1. SetByCaller 기본 데미지
	float BaseDamage = Spec.GetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, false, 0.f);
	if (BaseDamage <= 0.f)
	{
		return;
	}

	// 2. 부위 배율 (Body.WeakSpot / Body.ArmoredLimb)
	float PartMultiplier = 1.f;
	const float WeakSpot   = Spec.GetSetByCallerMagnitude(BlackoutGameplayTags::Body_WeakSpot,   false, -1.f);
	const float ArmoredLimb = Spec.GetSetByCallerMagnitude(BlackoutGameplayTags::Body_ArmoredLimb, false, -1.f);
	if (WeakSpot > 0.f)       PartMultiplier = WeakSpot;
	else if (ArmoredLimb > 0.f) PartMultiplier = ArmoredLimb;

	// 3. 타겟 피해 감소율
	float DamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetStatics().DamageReductionDef, EvalParams, DamageReduction);
	DamageReduction = FMath::Clamp(DamageReduction, 0.f, 1.f);

	// 4. 최종 피해 출력
	const float FinalDamage = BaseDamage * PartMultiplier * (1.f - DamageReduction);
	BO_LOG_GAS(Verbose, "DamageCalc: Base=%.1f Part=%.2f Reduction=%.2f Final=%.1f",
		BaseDamage, PartMultiplier, DamageReduction, FinalDamage);

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UBlackoutBaseAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-FinalDamage));
}
