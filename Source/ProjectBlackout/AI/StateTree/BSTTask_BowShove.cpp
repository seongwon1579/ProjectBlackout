#include "AI/StateTree/BSTTask_BowShove.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BlackoutGameplayTags.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

EStateTreeRunStatus FBSTTask_BowShove::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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

	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_BowShove));
	return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBSTTask_BowShove::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
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
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_BowShove), Specs);

	for (const FGameplayAbilitySpec* Spec : Specs)
	{
		if (Spec && Spec->IsActive())
		{
			return EStateTreeRunStatus::Running;
		}
	}

	return EStateTreeRunStatus::Succeeded;
}
