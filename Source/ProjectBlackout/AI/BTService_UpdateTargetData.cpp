#include "AI/BTService_UpdateTargetData.h"
#include "AI/BTNodeHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "KismetAnimationLibrary.h"

UBTService_UpdateTargetData::UBTService_UpdateTargetData()
{
	NodeName = "Update Target Data";
	Interval = 0.1f;
	RandomDeviation = 0.05f;
}

void UBTService_UpdateTargetData::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	//
	// UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	// APawn* AIPawn = UBTNodeHelper::GetAIPawn(OwnerComp);
	// //AActor* Target = UBTNodeHelper::GetActorFromBB(OwnerComp, TargetKey);
	// if (!BB || !AIPawn || !Target) return;
	//
	// const FVector AILocation = AIPawn->GetActorLocation();
	// const FVector TargetLocation = Target->GetActorLocation();
	// const FVector ToTarget = TargetLocation - AILocation;
	//
	// const float Distance = ToTarget.Size2D();
	// const float DirectionAngle = UKismetAnimationLibrary::CalculateDirection(ToTarget, AIPawn->GetActorRotation());
	//
	// BB->SetValueAsVector(OutLocationKey.SelectedKeyName, TargetLocation);
	// BB->SetValueAsFloat(OutDistanceKey.SelectedKeyName, Distance);
	// BB->SetValueAsFloat(OutDirectionAngleKey.SelectedKeyName, DirectionAngle);
}