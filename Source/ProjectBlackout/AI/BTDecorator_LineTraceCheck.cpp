// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTDecorator_LineTraceCheck.h"

#include "BTNodeHelper.h"
#include "DrawDebugHelpers.h"

bool UBTDecorator_LineTraceCheck::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	APawn* Owner = UBTNodeHelper::GetAIPawn(OwnerComp);
	if (!Owner) return false;

	FVector Start = Owner->GetActorLocation();
	FVector End = Start + Owner->GetActorForwardVector() * TraceLength;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	const bool bHit = Owner->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);

	const FColor LineColor = bHit ? FColor::Green : FColor::Red;
	DrawDebugLine(Owner->GetWorld(), Start, End, LineColor, false, 2.f, 0, 2.f);
	if (bHit)
	{
		DrawDebugSphere(Owner->GetWorld(), Hit.ImpactPoint, 10.f, 8, FColor::Yellow, false, -1.f);
	}

	return bHit;
}

FString UBTDecorator_LineTraceCheck::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}
