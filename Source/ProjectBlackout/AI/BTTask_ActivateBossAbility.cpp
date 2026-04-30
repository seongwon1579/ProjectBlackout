#include "AI/BTTask_ActivateBossAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagsManager.h"
#include "Abilities/GameplayAbility.h"

UBTTask_ActivateBossAbility::UBTTask_ActivateBossAbility()
{
	NodeName = "Activate Boss Ability";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ActivateBossAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI || !AI->GetPawn()) return EBTNodeResult::Failed;

	// ── 1. 태그 소스 결정 ────────────────────────────────────────────────────
	const FGameplayTag Tag = ResolveAbilityTag(OwnerComp);
	if (!Tag.IsValid()) return EBTNodeResult::Failed;

	// ── 2. 블랙보드에서 타겟 읽기 ────────────────────────────────────────────
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	APawn* BBTarget = BB ? Cast<APawn>(BB->GetValueAsObject(TEXT("BB_CurrentTarget"))) : nullptr;

	// ── 3. GA 실행 (타겟을 EventData에 담아 HandleGameplayEvent로 전달) ──────
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AI->GetPawn());
	if (!ASC) return EBTNodeResult::Failed;

	FGameplayEventData EventData;
	EventData.Target = BBTarget;

	// ── 4. GA 종료 대기 설정 (트리거 전에 바인딩) ────────────────────────────
	if (bWaitForEnd)
	{
		CachedOwnerComp = &OwnerComp;
		CachedASC       = ASC;
		ASC->AbilityActivatedCallbacks.AddUObject(this, &UBTTask_ActivateBossAbility::HandleAbilityActivated);
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

void UBTTask_ActivateBossAbility::HandleAbilityActivated(UGameplayAbility* Ability)
{
	// 일회성 — 첫 번째 활성화만 잡고 즉시 제거
	if (CachedASC.IsValid())
	{
		CachedASC->AbilityActivatedCallbacks.RemoveAll(this);
	}

	BoundAbility = Ability;
	Ability->OnGameplayAbilityEnded.AddUObject(this, &UBTTask_ActivateBossAbility::HandleAbilityEnded);
}

void UBTTask_ActivateBossAbility::HandleAbilityEnded(UGameplayAbility* Ability)
{
	UnbindDelegate();
	if (CachedOwnerComp)
	{
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
		CachedOwnerComp = nullptr;
	}
}

EBTNodeResult::Type UBTTask_ActivateBossAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UnbindDelegate();
	CachedOwnerComp = nullptr;
	return EBTNodeResult::Aborted;
}

void UBTTask_ActivateBossAbility::UnbindDelegate()
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

FString UBTTask_ActivateBossAbility::GetStaticDescription() const
{
	if (bReadTagFromBlackboard)
	{
		return FString::Printf(TEXT("Activate GA from BB: %s"), *AbilityTagKey.SelectedKeyName.ToString());
	}
	return FString::Printf(TEXT("Activate GA: %s"), *AbilityTag.ToString());
}

FGameplayTag UBTTask_ActivateBossAbility::ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const
{
	if (!bReadTagFromBlackboard)
	{
		return AbilityTag;
	}

	if (AbilityTagKey.SelectedKeyName.IsNone())
	{
		UE_LOG(LogTemp, Error, TEXT("ActivateBossAbility: AbilityTagKey가 설정되지 않았습니다. 에디터에서 BB 키를 선택하세요."));
		return FGameplayTag::EmptyTag;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return FGameplayTag::EmptyTag;

	const FName TagName = BB->GetValueAsName(AbilityTagKey.SelectedKeyName);
	if (TagName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateBossAbility: BB 키[%s]에서 읽은 태그 이름이 None입니다. BTTask_SelectPattern이 먼저 실행됐는지 확인하세요."),
			*AbilityTagKey.SelectedKeyName.ToString());
		return FGameplayTag::EmptyTag;
	}

	const FGameplayTag Tag = UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);
	if (!Tag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ActivateBossAbility: [%s]는 등록된 GameplayTag가 아닙니다. 태그 테이블을 확인하세요."),
			*TagName.ToString());
	}

	return Tag;
}
