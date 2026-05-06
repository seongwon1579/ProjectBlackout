// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTD_DistanceDeviationExceeded.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTD_DistanceDeviationExceeded::UBTD_DistanceDeviationExceeded()
{
    NodeName = TEXT("Is Chase State");
    bCreateNodeInstance = true;

    //     static ConstructorHelpers::FObjectFinder<UEnum> EnumFinder(
    //         TEXT("/Game/_BP/Enemy/AI/BehaviourTree/EAI_State.EAI_State"));
    //     BlackboardKey.AddEnumFilter(
    //         this,
    //         GET_MEMBER_NAME_CHECKED(UBTD_DistanceDeviationExceeded, BlackboardKey),
    //         EnumFinder.Succeeded() ? EnumFinder.Object : nullptr);
}

void UBTD_DistanceDeviationExceeded::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
    CachedOwnerComp = &OwnerComp;
    bHasLastResult = false;
}

bool UBTD_DistanceDeviationExceeded::CalculateRawConditionValue(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return false;

    // const uint8 CurrentState = BB->GetValueAsEnum(BlackboardKey.SelectedKeyName);
    //
    // return CurrentState == ChaseStateIndex;
    return BB->GetValueAsBool(BlackboardKey.SelectedKeyName);
}

EBlackboardNotificationResult UBTD_DistanceDeviationExceeded::OnBlackboardKeyValueChange(
    const UBlackboardComponent& Blackboard, FBlackboard::FKey ChangedKeyID)
{
    UE_LOG(LogTemp, Warning, TEXT("Changed"));
    
    if (!CachedOwnerComp.IsValid())
    {
        return EBlackboardNotificationResult::RemoveObserver;
    }

    if (BlackboardKey.GetSelectedKeyID() != ChangedKeyID)
    {
        return EBlackboardNotificationResult::ContinueObserving;
    }

    UBehaviorTreeComponent* BTComp = CachedOwnerComp.Get();
    const bool bNewResult = CalculateRawConditionValue(*BTComp, nullptr);
    if (!bHasLastResult || bNewResult != bLastResult)
    {
        bLastResult = bNewResult;
        bHasLastResult = true;

        UE_LOG(LogTemp, Warning, TEXT("재평가: %s"), bNewResult ? TEXT("True") : TEXT("False"));
        BTComp->RequestExecution(this);
    }

    return EBlackboardNotificationResult::ContinueObserving;
}
