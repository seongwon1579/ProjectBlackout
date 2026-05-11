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
	
	// Cooldown 중이면 Failed — ST가 다른 State(Kite)로 트랜지션. BowShove / Teleport 패턴 통일.
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows));
	return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
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

	for (const FGameplayAbilitySpec* Spec : Specs)
	{
		if (Spec && Spec->IsActive())
		{
			return EStateTreeRunStatus::Running;
		}
	}
	return EStateTreeRunStatus::Succeeded;
}
