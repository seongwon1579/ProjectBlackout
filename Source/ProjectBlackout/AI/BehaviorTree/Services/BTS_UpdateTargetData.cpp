//#include "AI/BehaviorTree/Services/UBTS_UpdateTargetData.h"
#include "AI/BehaviorTree/Services/BTS_UpdateTargetData.h"

#include "AI/BehaviorTree/BTNodeHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BlackoutBossCharacter.h"
#include "KismetAnimationLibrary.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Float.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"

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
	
	if (!OwnerComp.IsRunning()) return;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;
	
	ABlackoutBossCharacter* Owner = UBTNodeHelper::GetAIPawn<ABlackoutBossCharacter>(OwnerComp);
	if (!Owner) return;
	
	const APawn* Target = Cast<const APawn>(BB->GetValue<UBlackboardKeyType_Object>(TargetKey.GetSelectedKeyID()));
	if (!Target) return;
	
	const FVector OwnerLocation = Owner->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();
	const FVector ToTarget = TargetLocation - OwnerLocation;
	
	BB->SetValue<UBlackboardKeyType_Float>(DistanceBTWActorsKey.GetSelectedKeyID(), ToTarget.Size2D());
}

void UBTS_UpdateTargetData::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	
	if (auto BBAsset = GetBlackboardAsset())
	{
		TargetKey.ResolveSelectedKey(*BBAsset);
		DistanceBTWActorsKey.ResolveSelectedKey(*BBAsset);
	}
}
