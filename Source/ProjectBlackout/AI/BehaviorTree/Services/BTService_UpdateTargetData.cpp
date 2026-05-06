#include "AI/BehaviorTree/Services/BTService_UpdateTargetData.h"

#include "AIController.h"
#include "AI/BOAggroComponent.h"
#include "AI/BehaviorTree/BTNodeHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BlackoutBossCharacter.h"
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
	
	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	if (!BB) return;
	
	// AggroComponent에서 최고 위협 타겟 가져오기
	ABlackoutBossCharacter* Enemy = UBTNodeHelper::GetAIPawn<ABlackoutBossCharacter>(OwnerComp);
	if (!Enemy) return;
	
	// APawn* CurrentTarget = Enemy && Enemy->AggroComponent
	// 	? Enemy->AggroComponent->GetHighestAggroTarget()
	// 	: nullptr;
	
	APawn* CurrentTarget  = GetWorld()->GetFirstPlayerController()->GetPawn();
	
	if (!CurrentTarget) return;
	
	const FVector AILocation     = Enemy->GetActorLocation();
	const FVector TargetLocation = CurrentTarget->GetActorLocation();
	const FVector ToTarget       = TargetLocation - AILocation;

	BB->SetValueAsObject(CurrentTargetKey.SelectedKeyName, CurrentTarget);
	BB->SetValueAsVector(CurrentTargetLocationKey.SelectedKeyName,     TargetLocation);
	BB->SetValueAsFloat(DistanceBTWActorsKey.SelectedKeyName,      ToTarget.Size2D());
	BB->SetValueAsFloat(AngleBTWActorsKey.SelectedKeyName,
		UKismetAnimationLibrary::CalculateDirection(ToTarget, Enemy->GetActorRotation()));
}
