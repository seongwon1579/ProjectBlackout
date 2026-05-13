#include "AI/BehaviorTree/Tasks/BTT_ActivateAbility.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagsManager.h"
#include "Abilities/GameplayAbility.h"
#include "BehaviorTree/BTNodeHelper.h"

UBTT_ActivateAbility::UBTT_ActivateAbility()
{
	NodeName = "Activate Boss Ability";
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
	
	APawn* Target = BB ? Cast<APawn>(BB->GetValueAsObject(CurrentTargetKey.SelectedKeyName)) : nullptr;
	if (!Target) return EBTNodeResult::Failed;

	// ── 3. GA 실행 (타겟을 EventData에 담아 HandleGameplayEvent로 전달) ──────
	UAbilitySystemComponent* ASC = UBTNodeHelper::GetAbilitySystemComponent(OwnerComp);
	if (!ASC) return EBTNodeResult::Failed;

	FGameplayEventData EventData;
	EventData.Target = Target;
	
	if (bPassSignedAngle)
	{
		EventData.EventMagnitude = BB->GetValueAsFloat(SignedAngleKey.SelectedKeyName);
	}

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
	if (bReadTagFromBlackboard)
	{
		return FString::Printf(TEXT("Activate GA from BB: %s"), *SelectedGameAbilityTagKey.SelectedKeyName.ToString());
	}
	return FString::Printf(TEXT("Activate GA: %s"), *AbilityTag.ToString());
}

FGameplayTag UBTT_ActivateAbility::ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const
{
	if (!bReadTagFromBlackboard)
	{
		return AbilityTag;
	}

	if (SelectedGameAbilityTagKey.SelectedKeyName.IsNone())
	{
		return FGameplayTag::EmptyTag;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return FGameplayTag::EmptyTag;

	const FName TagName = BB->GetValueAsName(SelectedGameAbilityTagKey.SelectedKeyName);
	if (TagName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("ActivateBossAbility: BB 키[%s]에서 읽은 태그 이름이 None입니다. BTTask_SelectPattern이 필요합니다."),
			*SelectedGameAbilityTagKey.SelectedKeyName.ToString());
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
