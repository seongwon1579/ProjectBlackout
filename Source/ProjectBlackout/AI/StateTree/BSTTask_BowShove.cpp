#include "AI/StateTree/BSTTask_BowShove.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BlackoutGameplayTags.h"
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

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.OwnerCharacter);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_BowShove));
	return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBSTTask_BowShove::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// TODO: 후퇴 이동(PostShoveBackstepDistance) 완료 여부 확인
	//       완료 조건 달성 시 Succeeded 반환, 미완료 시 Running 유지
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.OwnerCharacter)
	{
		return EStateTreeRunStatus::Failed;
	}
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.OwnerCharacter);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	TArray<FGameplayAbilitySpec*> Specs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_BowShove), Specs);
	
	for (const FGameplayAbilitySpec* Spec : Specs)
	{
		if ( Spec && Spec->IsActive())
		{
			return EStateTreeRunStatus::Running;
		}
	}
	
	return EStateTreeRunStatus::Succeeded;
}
