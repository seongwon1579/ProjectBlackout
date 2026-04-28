#include "AI/BSTTask_FireTwinArrows.h"
#include "StateTreeExecutionContext.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "Combat/Weapons/BOProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BlackoutGameplayTags.h"

EStateTreeRunStatus FBSTTask_FireTwinArrows::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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
	
	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows));
	

	return bActivated ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBSTTask_FireTwinArrows::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
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
	
	TArray<FGameplayAbilitySpec*> Specs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(BlackoutGameplayTags::Ability_Wraith_FireTwinArrows), Specs);
	
	for (const FGameplayAbilitySpec* Spec : Specs)
	{
		if ( Spec && Spec->IsActive())
		{
			return EStateTreeRunStatus::Running;
		}
	}
	
	return EStateTreeRunStatus::Succeeded;
}
