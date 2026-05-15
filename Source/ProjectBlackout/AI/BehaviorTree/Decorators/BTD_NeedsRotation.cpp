// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_NeedsRotation.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTD_NeedsRotation::UBTD_NeedsRotation()
{
	NodeName = TEXT("NeedsRotation");
}

bool UBTD_NeedsRotation::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	auto BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	float SignedAngle = BB->GetValueAsFloat(SignedAngleKey.SelectedKeyName);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<uint64>(GetUniqueID()),
			0.0f,
			FMath::Abs(SignedAngle) >= AngleThreshold ? FColor::Red : FColor::Green,
			FString::Printf(TEXT("[NeedsRotation] Angle: %.1f / Threshold: %.1f"), SignedAngle, AngleThreshold)
		);
	}

	return FMath::Abs(SignedAngle) >= AngleThreshold;
}

FString UBTD_NeedsRotation::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
