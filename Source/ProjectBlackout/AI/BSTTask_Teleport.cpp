#include "AI/BSTTask_Teleport.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BlackoutGameplayTags.h"
#include "StateTreeExecutionContext.h"
#include "Characters/BlackoutEnemyCharacter.h"

EStateTreeRunStatus FBSTTask_Teleport::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
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
	
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_Teleport));
	return bActivated ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}
