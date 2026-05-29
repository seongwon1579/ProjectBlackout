#include "AI/StateTree/BSTTask_ActivateAbility.h"
#include "StateTreeExecutionContext.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

EStateTreeRunStatus FBSTTask_ActivateAbility::EnterState(FStateTreeExecutionContext& Context,
                                                         const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& Data = Context.GetInstanceData(*this);
    Data.ActiveHandle = FGameplayAbilitySpecHandle();

    if (!Data.OwnerPawn || !Data.AbilityTag.IsValid())
    {
       return EStateTreeRunStatus::Failed;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Data.OwnerPawn);
    if (!ASC)
    {
       return EStateTreeRunStatus::Failed;
    }

    TArray<FGameplayAbilitySpec*> Specs;
    ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(Data.AbilityTag), Specs);
    if (Specs.Num() == 0)
    {
       return EStateTreeRunStatus::Failed; 
    }

    FGameplayEventData EventData;
    EventData.Instigator = Data.OwnerPawn;
    EventData.Target = Data.TargetActor; 

    int32 TriggeredCount = ASC->HandleGameplayEvent(Data.AbilityTag, &EventData);
    
    if (TriggeredCount > 0)
    {
       for (FGameplayAbilitySpec* Spec : Specs)
       {
          if (Spec && Spec->IsActive())
          {
             Data.ActiveHandle = Spec->Handle;
             return EStateTreeRunStatus::Running;
          }
       }
       
       return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FBSTTask_ActivateAbility::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    const FInstanceDataType& Data = Context.GetInstanceData(*this);
    
    if (!Data.OwnerPawn || !Data.ActiveHandle.IsValid())
    {
       return EStateTreeRunStatus::Failed;
    }

    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Data.OwnerPawn);
    if (!ASC)
    {
       return EStateTreeRunStatus::Failed;
    }

    FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Data.ActiveHandle);
    if (Spec && Spec->IsActive())
    {
       return EStateTreeRunStatus::Running;
    }

    return EStateTreeRunStatus::Succeeded;
}

void FBSTTask_ActivateAbility::ExitState(FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{
    const FInstanceDataType& Data = Context.GetInstanceData(*this);

    if (!Data.OwnerPawn || !Data.ActiveHandle.IsValid())
    {
       return;
    }

    if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Data.OwnerPawn))
    {
       FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Data.ActiveHandle);
       if (Spec && Spec->IsActive())
       {
          ASC->CancelAbilityHandle(Data.ActiveHandle);
       }
    }
}