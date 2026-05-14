
// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_CanEvadeSide.h"

#include "DrawDebugHelpers.h"
#include "BehaviorTree/BTNodeHelper.h"

bool UBTD_CanEvadeSide::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* Owner = UBTNodeHelper::GetAIPawn(OwnerComp);
	if (!Owner) return false;

	FVector Start = Owner->GetActorLocation();
	// FVector End = Start + Owner->GetActorForwardVector() * TraceLength;
	FVector End = Start + Owner->GetActorRightVector() * TraceLength;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	const float CapsuleRadius = Owner->GetSimpleCollisionRadius();

	const bool bHit = Owner->GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(CapsuleRadius),
		Params
	);
	
#if ENABLE_DRAW_DEBUG
	const FColor LineColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugCapsule(
		Owner->GetWorld(),
		(Start + End) * 0.5f,           
		TraceLength * 0.5f,           
		CapsuleRadius,                   
		FRotationMatrix::MakeFromZ(End - Start).ToQuat(),
		LineColor,
		false,
		0.2f                             
	);
	if (bHit)
	{
		DrawDebugSphere(Owner->GetWorld(), Hit.ImpactPoint, 10.f, 8, FColor::Yellow, false, 0.2f);
	}
#endif

	return bHit;
}

FString UBTD_CanEvadeSide::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
