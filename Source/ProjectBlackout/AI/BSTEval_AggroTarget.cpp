#include "AI/BSTEval_AggroTarget.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemComponent.h"
#include "AI/BlackoutBossAIController.h"
#include "Data/BOBossData.h"
#include "GameFramework/PlayerState.h"

void FBSTEval_AggroTarget::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	if (!InstanceData.OwnerASC || InstanceData.OwnerASC->GetOwner()->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	InstanceData.DamageAccumulator.Empty();
	InstanceData.CurrentTargetPS = nullptr;
	InstanceData.LastSwitchTime = 0.0f;

	// TODO: 바인딩
}

void FBSTEval_AggroTarget::TreeStop(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.OwnerASC && InstanceData.OwnerASC->GetOwner()->GetLocalRole() == ROLE_Authority)
	{
		// TODO: 바인딩 해제
	}
	InstanceData.DamageAccumulator.Empty();
}

void FBSTEval_AggroTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.OwnerASC || InstanceData.OwnerASC->GetOwner()->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// 1. 누적 피해 감쇠
	if (InstanceData.BossData)
	{
		float Decay = InstanceData.BossData->AggroDecayRate * DeltaTime;
		for (auto& Pair : InstanceData.DamageAccumulator)
		{
			Pair.Value = FMath::Max(0.0f, Pair.Value - Pair.Value * Decay);
		}
	}

	// 2. 타겟 선정
	APlayerState* BestTarget = nullptr;

	// 3. 기록
	if (BestTarget && InstanceData.Controller)
	{
		APawn* TargetPawn = BestTarget->GetPawn();
		InstanceData.OutTarget = TargetPawn;
		InstanceData.Controller->WriteTargetToBlackboard(TargetPawn);
	}
}
