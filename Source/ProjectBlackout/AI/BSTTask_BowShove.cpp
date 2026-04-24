#include "AI/BSTTask_BowShove.h"
#include "AI/BOAICalcHelper.h"
#include "StateTreeExecutionContext.h"
#include "Characters/BORootWraith.h"

EStateTreeRunStatus FBSTTask_BowShove::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.OwnerCharacter || !InstanceData.TargetPawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	// 사거리 초과 시 즉시 실패 — Kite 상태로 되돌아가도록 StateTree 전이
	if (!UBOAICalcHelper::IsWithinRange(InstanceData.OwnerCharacter, InstanceData.TargetPawn, InstanceData.ShoveRange))
	{
		return EStateTreeRunStatus::Failed;
	}

	// TODO: OwnerCharacter->BowShove(InstanceData.TargetPawn) 호출
	//       — 임펄스(ShoveImpulse) 방향 계산 및 LaunchCharacter 적용
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBSTTask_BowShove::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// TODO: 후퇴 이동(PostShoveBackstepDistance) 완료 여부 확인
	//       완료 조건 달성 시 Succeeded 반환, 미완료 시 Running 유지
	return EStateTreeRunStatus::Succeeded;
}
