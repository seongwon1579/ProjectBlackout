// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_CanEvade.h"

#include "DrawDebugHelpers.h"
#include "BehaviorTree/BTNodeHelper.h"
#include "BehaviorTree/Enum/EvadeDirection.h"

bool UBTD_CanEvade::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const APawn* Owner = UBTNodeHelper::GetAIPawn(OwnerComp);
	if (!Owner) return false;

	auto* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;
	
	bool bCanRight = !IsBlocked(Owner, Owner->GetActorRightVector());
	bool bCanLeft = !IsBlocked(Owner, -Owner->GetActorRightVector());
	
	if (!bCanLeft && !bCanRight) return false;
	
	EEvadeDirection Dir;
	if (bCanLeft && bCanRight)
	{
		Dir = FMath::RandBool() ? EEvadeDirection::Right : EEvadeDirection::Left;
	}
	else
	{
		Dir = bCanRight ? EEvadeDirection::Right : EEvadeDirection::Left;
	}
	
	BB->SetValueAsEnum(EvadeDirectionKey.SelectedKeyName, static_cast<uint8>(Dir));
	
	return true;
}

FString UBTD_CanEvade::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}

bool UBTD_CanEvade::IsBlocked(const APawn* Owner, const FVector& Direction) const
{
	const FVector Start = Owner->GetActorLocation();
	const FVector End = Start + Direction * TraceLength;
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	
	auto* World = Owner->GetWorld();
	if (!World) return false;
	
	const float CapsuleRadius = Owner->GetSimpleCollisionRadius();
	
	bool bHit = World->SweepSingleByChannel(
		Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(CapsuleRadius), Params);
	
#if ENABLE_DRAW_DEBUG
	if (bEnableDebugDraw)
	{
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
	}
#endif
	
	return bHit;
}
