//#include "AI/BehaviorTree/Services/UBTS_UpdateTargetData.h"
#include "AI/BehaviorTree/Services/BTS_UpdateTargetData.h"

#include "AI/BehaviorTree/BTNodeHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BlackoutBossCharacter.h"
#include "KismetAnimationLibrary.h"

UBTS_UpdateTargetData::UBTS_UpdateTargetData()
{
	NodeName = "Update Target Data";
	Interval = 0.1f;

	bNotifyTick = true;
	bCreateNodeInstance = true;
}

void UBTS_UpdateTargetData::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	UpdateBlackboard(OwnerComp);
}

void UBTS_UpdateTargetData::UpdateBlackboard(UBehaviorTreeComponent& OwnerComp)
{
	if (!OwnerComp.IsRunning()) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	// AggroComponent에서 최고 위협 타겟 가져오기
	ABlackoutBossCharacter* Owner = UBTNodeHelper::GetAIPawn<ABlackoutBossCharacter>(OwnerComp);
	if (!Owner) return;

	// APawn* CurrentTarget = Enemy && Enemy->AggroComponent
	// 	? Enemy->AggroComponent->GetHighestAggroTarget()
	// 	: nullptr;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Target = PC ? PC->GetPawn() : nullptr;

	if (!Target) return;

	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector ToTarget = TargetLocation - OwnerLocation;

	BB->SetValueAsObject(TargetKey.SelectedKeyName, Target);
	BB->SetValueAsVector(TargetLocationKey.SelectedKeyName, TargetLocation);
	BB->SetValueAsFloat(DistanceBTWActorsKey.SelectedKeyName, ToTarget.Size2D());
	BB->SetValueAsFloat(AngleBTWActorsKey.SelectedKeyName,
	                    UKismetAnimationLibrary::CalculateDirection(ToTarget, Owner->GetActorRotation()));
}
