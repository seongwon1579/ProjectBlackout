#include "AI/StateTree/BSTTask_FireTwinArrows.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BlackoutGameplayTags.h"

EStateTreeRunStatus FBSTTask_FireTwinArrows::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.OwnerPawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.OwnerPawn);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	// 활성화 결과 무관하게 Running — Tick 이 재시도 책임 (Cooldown 진행 중에 Combat 진입 시에도 stuck 안 됨)
	ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBSTTask_FireTwinArrows::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.OwnerPawn)
	{
		return EStateTreeRunStatus::Failed;
	}
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstanceData.OwnerPawn);
	if (!ASC)
	{
		return EStateTreeRunStatus::Failed;
	}
	
	TArray<FGameplayAbilitySpec*> Specs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows), Specs);
	
	bool bAnyActive = false;
	for (const FGameplayAbilitySpec* Spec : Specs)
	{
		if (Spec && Spec->IsActive())
		{
			bAnyActive = true;
			break;
		}
	}

	// 비활성이면 재활성화 시도 — Cooldown GE 가 페이싱 결정 (쿨다운 중이면 ASC 거부)
	if (!bAnyActive)
	{
		ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows));
	}

	return EStateTreeRunStatus::Running;
}
