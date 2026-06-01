#include "AI/BehaviorTree/Tasks/BTT_ActivateAbility.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagsManager.h"
#include "Abilities/GameplayAbility.h"
#include "BehaviorTree/BTNodeHelper.h"

UBTT_ActivateAbility::UBTT_ActivateAbility()
{
	NodeName = TEXT("Activate Ability");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// ── 태그 소스 결정 ────────────────────────────────────────────────────
	const FGameplayTag Tag = ResolveAbilityTag(OwnerComp);
	if (!Tag.IsValid()) return EBTNodeResult::Failed;

	// ── 블랙보드에서 타겟 읽기 ────────────────────────────────────────────
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;
	
	APawn* Target = BB ? Cast<APawn>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;
	if (!Target) return EBTNodeResult::Failed;

	// ── 3. GA 실행 (타겟을 EventData에 담아 HandleGameplayEvent로 전달) ──────
	UAbilitySystemComponent* ASC = UBTNodeHelper::GetAbilitySystemComponent(OwnerComp);
	if (!ASC) return EBTNodeResult::Failed;

	FGameplayEventData EventData;
	EventData.Target = Target;
	
	PrepareEventData(EventData, BB);

	// ── 4. GA 종료 대기 설정  ────────────────────────────
	if (bWaitForEnd)
	{
		CachedOwnerComp = &OwnerComp;
		CachedASC       = ASC;
		ASC->AbilityActivatedCallbacks.AddUObject(this, &UBTT_ActivateAbility::HandleAbilityActivated);
	}

	const int32 TriggeredCount = ASC->HandleGameplayEvent(Tag, &EventData);
	if (TriggeredCount == 0)
	{
		UnbindDelegate();
		CachedOwnerComp = nullptr;
		return EBTNodeResult::Failed;
	}

	if (!bWaitForEnd) return EBTNodeResult::Succeeded;
	return EBTNodeResult::InProgress;
}

void UBTT_ActivateAbility::HandleAbilityActivated(UGameplayAbility* Ability)
{
	if (CachedASC.IsValid())
	{
		CachedASC->AbilityActivatedCallbacks.RemoveAll(this);
	}

	BoundAbility = Ability;
	Ability->OnGameplayAbilityEnded.AddUObject(this, &UBTT_ActivateAbility::HandleAbilityEnded);
}

void UBTT_ActivateAbility::HandleAbilityEnded(UGameplayAbility* Ability)
{
	UnbindDelegate();
	if (CachedOwnerComp)
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		CachedOwnerComp = nullptr;
	}
}

EBTNodeResult::Type UBTT_ActivateAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnbindDelegate();
	CachedOwnerComp = nullptr;
	return EBTNodeResult::Aborted;
}

void UBTT_ActivateAbility::UnbindDelegate()
{
	if (CachedASC.IsValid())
	{
		CachedASC->AbilityActivatedCallbacks.RemoveAll(this);
	}
	CachedASC.Reset();

	if (BoundAbility.IsValid())
	{
		BoundAbility->OnGameplayAbilityEnded.RemoveAll(this);
	}
	BoundAbility.Reset();
}

FString UBTT_ActivateAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Activate GA: %s"), *AbilityTag.ToString());
}

FGameplayTag UBTT_ActivateAbility::ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const
{
	if (AbilityTag.IsValid())
	{
		return AbilityTag;
	}
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return FGameplayTag();

	const FName TagName = BB->GetValueAsName(SelectedAbilityTagKey.SelectedKeyName);
	if (TagName.IsNone()) return FGameplayTag();

	return FGameplayTag::RequestGameplayTag(TagName, false); 
}
