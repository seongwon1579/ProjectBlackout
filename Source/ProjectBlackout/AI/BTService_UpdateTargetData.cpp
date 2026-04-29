#include "AI/BTService_UpdateTargetData.h"

#include "AIController.h"
#include "AI/BOAggroComponent.h"
#include "AI/BTNodeHelper.h"
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

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!BB || !AI) return;

	APawn* AIPawn = AI->GetPawn();
	if (!AIPawn) return;

	// AggroComponent에서 최고 위협 타겟 가져오기
	ABlackoutBossCharacter* Boss = Cast<ABlackoutBossCharacter>(AIPawn);
	APawn* AggroTarget = Boss && Boss->AggroComponent
		? Boss->AggroComponent->GetHighestAggroTarget()
		: nullptr;

	if (AggroTarget)
	{
		BB->SetValueAsObject(CurrentTargetKey.SelectedKeyName, AggroTarget);
	}

	// 현재 타겟 기준으로 위치/거리/방향 갱신
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(CurrentTargetKey.SelectedKeyName));
	if (!Target) return;

	const FVector AILocation     = AIPawn->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector ToTarget       = TargetLocation - AILocation;

	BB->SetValueAsVector(OutLocationKey.SelectedKeyName,     TargetLocation);
	BB->SetValueAsFloat(OutDistanceKey.SelectedKeyName,      ToTarget.Size2D());
	BB->SetValueAsFloat(OutDirectionAngleKey.SelectedKeyName,
		UKismetAnimationLibrary::CalculateDirection(ToTarget, AIPawn->GetActorRotation()));
}
