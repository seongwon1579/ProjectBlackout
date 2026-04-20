#include "GAS/Effects/ExecCalc_Reload.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "BlackoutGameplayTags.h"

namespace
{
	struct FReloadCalcStatics
	{
		DECLARE_ATTRIBUTE_CAPTUREDEF(PrimaryClipAmmo);
		DECLARE_ATTRIBUTE_CAPTUREDEF(PrimaryMaxClip);
		DECLARE_ATTRIBUTE_CAPTUREDEF(PrimaryReserveAmmo);
		
		DECLARE_ATTRIBUTE_CAPTUREDEF(SecondaryClipAmmo);
		DECLARE_ATTRIBUTE_CAPTUREDEF(SecondaryMaxClip);
		DECLARE_ATTRIBUTE_CAPTUREDEF(SecondaryReserveAmmo);

		FReloadCalcStatics()
		{
			DEFINE_ATTRIBUTE_CAPTUREDEF(UBlackoutAmmoAttributeSet, PrimaryClipAmmo, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UBlackoutAmmoAttributeSet, PrimaryMaxClip, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UBlackoutAmmoAttributeSet, PrimaryReserveAmmo, Target, false);
			
			DEFINE_ATTRIBUTE_CAPTUREDEF(UBlackoutAmmoAttributeSet, SecondaryClipAmmo, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UBlackoutAmmoAttributeSet, SecondaryMaxClip, Target, false);
			DEFINE_ATTRIBUTE_CAPTUREDEF(UBlackoutAmmoAttributeSet, SecondaryReserveAmmo, Target, false);
		}
	};

	const FReloadCalcStatics& GetReloadStatics()
	{
		static FReloadCalcStatics Statics;
		return Statics;
	}
}

UExecCalc_Reload::UExecCalc_Reload()
{
	RelevantAttributesToCapture.Add(GetReloadStatics().PrimaryClipAmmoDef);
	RelevantAttributesToCapture.Add(GetReloadStatics().PrimaryMaxClipDef);
	RelevantAttributesToCapture.Add(GetReloadStatics().PrimaryReserveAmmoDef);
	
	RelevantAttributesToCapture.Add(GetReloadStatics().SecondaryClipAmmoDef);
	RelevantAttributesToCapture.Add(GetReloadStatics().SecondaryMaxClipDef);
	RelevantAttributesToCapture.Add(GetReloadStatics().SecondaryReserveAmmoDef);
}

void UExecCalc_Reload::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const bool bIsPrimary = !Spec.CapturedSourceTags.GetAggregatedTags()->HasTagExact(BlackoutGameplayTags::Weapon_Secondary);

	float ClipAmmo = 0.0f;
	float MaxClip = 0.0f;
	float ReserveAmmo = 0.0f;

	if (bIsPrimary)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetReloadStatics().PrimaryClipAmmoDef, EvalParams, ClipAmmo);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetReloadStatics().PrimaryMaxClipDef, EvalParams, MaxClip);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetReloadStatics().PrimaryReserveAmmoDef, EvalParams, ReserveAmmo);
	}
	else
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetReloadStatics().SecondaryClipAmmoDef, EvalParams, ClipAmmo);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetReloadStatics().SecondaryMaxClipDef, EvalParams, MaxClip);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetReloadStatics().SecondaryReserveAmmoDef, EvalParams, ReserveAmmo);
	}

	float Missing = FMath::Max(0.0f, MaxClip - ClipAmmo);
	if (Missing <= 0.0f || ReserveAmmo <= 0.0f)
	{
		// 보충할 탄약이 없거나 예비탄이 없으면 종료
		return;
	}

	float Grant = FMath::Min(Missing, ReserveAmmo);

	if (bIsPrimary)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute(), EGameplayModOp::Additive, Grant));
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute(), EGameplayModOp::Additive, -Grant));
	}
	else
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute(), EGameplayModOp::Additive, Grant));
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute(), EGameplayModOp::Additive, -Grant));
	}
}
