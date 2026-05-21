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

    // 초기 페이즈 시작
    RequestPhaseChange(EBOBossPhase::Phase1);
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
    if (NewPhase == EBOBossPhase::None || NewPhase <= CurrentPhase) return;
    if (PendingPhase != EBOBossPhase::None && NewPhase <= PendingPhase) return;

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
    if (PendingPhase == EBOBossPhase::None || IsPhaseTransitionLocked()) return;

    const EBOBossPhase PhaseToApply = PendingPhase;
    PendingPhase = EBOBossPhase::None;

    ApplyPhaseChange(PhaseToApply);
}

void UBlackoutPhaseEvaluator::ApplyPhaseChange(EBOBossPhase NewPhase)
{
    CurrentPhase = NewPhase;
    
    if (OnBossPhaseChanged.IsBound())
    {
        OnBossPhaseChanged.Broadcast(NewPhase);
    }
}

bool UBlackoutPhaseEvaluator::IsPhaseTransitionLocked() const
{
    return CachedASC ? CachedASC->HasMatchingGameplayTag(PhaseLockTag) : false;
}