#include "AI/BlackoutPhaseEvaluator.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "BlackoutGameplayTags.h"

UBlackoutPhaseEvaluator::UBlackoutPhaseEvaluator() {}

UWorld* UBlackoutPhaseEvaluator::GetWorld() const
{
    return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

void UBlackoutPhaseEvaluator::Initialize(AAIController* InAIController, UAbilitySystemComponent* InASC)
{
    CachedOwnerAIController = InAIController;
    CachedASC = InASC;

    if (CachedASC)
    {
        PhaseLockTag = BlackoutGameplayTags::Ability_PhaseLock;
        PhaseLockTagChangedHandle = CachedASC->RegisterGameplayTagEvent(
            PhaseLockTag,
            EGameplayTagEventType::NewOrRemoved
        ).AddUObject(this, &UBlackoutPhaseEvaluator::OnPhaseLockTagChanged);
    }

    UE_LOG(LogTemp, Warning, TEXT("UBlackoutPhaseEvaluator Initialize"));
    // 초기 페이즈 시작
    //RequestPhaseChange(EBOBossPhase::Phase1);
}

void UBlackoutPhaseEvaluator::Deinitialize()
{
    if (CachedASC)
    {
        CachedASC->RegisterGameplayTagEvent(PhaseLockTag, EGameplayTagEventType::NewOrRemoved).Remove(PhaseLockTagChangedHandle);
    }
    CachedASC = nullptr;
    CachedOwnerAIController = nullptr;
    OnBossPhaseChanged.Clear();
}

void UBlackoutPhaseEvaluator::RequestPhaseChange(EBOBossPhase NewPhase)
{
    if (NewPhase == EBOBossPhase::None || NewPhase <= CurrentPhase)
    {
        UE_LOG(LogTemp, Warning, TEXT("RequestPhaseChange NewPhase == EBOBossPhase::None || NewPhase <= CurrentPhase "));
        return;
    }
    if (PendingPhase != EBOBossPhase::None && NewPhase <= PendingPhase)
    {
        UE_LOG(LogTemp, Warning, TEXT("PendingPhase != EBOBossPhase::None && NewPhase <= PendingPhase "));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("RequestPhaseChange Accepted! NewPhase: %d"), (int32)NewPhase);

    PendingPhase = NewPhase;
    TryApplyPendingPhase();
}

void UBlackoutPhaseEvaluator::OnPhaseLockTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    if (NewCount == 0)
    {
        TryApplyPendingPhase();
    }
}

void UBlackoutPhaseEvaluator::TryApplyPendingPhase()
{
    if (PendingPhase == EBOBossPhase::None || IsPhaseTransitionLocked())
    {
        UE_LOG(LogTemp, Warning, TEXT("TryApplyPendingPhase PendingPhase == EBOBossPhase::None || IsPhaseTransitionLocked() "));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("TryApplyPendingPhase"));
    const EBOBossPhase PhaseToApply = PendingPhase;
    PendingPhase = EBOBossPhase::None;

    ApplyPhaseChange(PhaseToApply);
}

void UBlackoutPhaseEvaluator::ApplyPhaseChange(EBOBossPhase NewPhase)
{
    CurrentPhase = NewPhase;
    
    UE_LOG(LogTemp, Warning, TEXT("ApplyPhaseChange"));
    
    // if (OnBossPhaseChanged.IsBound())
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("ApplyPhaseChange IsBound and BroadCast"));
    //     OnBossPhaseChanged.Broadcast(NewPhase);
    // }
    OnBossPhaseChanged.Broadcast(NewPhase);
}

bool UBlackoutPhaseEvaluator::IsPhaseTransitionLocked() const
{
    return CachedASC ? CachedASC->HasMatchingGameplayTag(PhaseLockTag) : false;
}