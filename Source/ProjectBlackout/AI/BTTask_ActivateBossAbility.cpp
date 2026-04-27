#include "AI/BTTask_ActivateBossAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/ActionPipeline.h"
#include "AI/ActionPipelineOwner.h"
#include "AI/FActionData.h"
#include "Abilities/GameplayAbility.h"

UBTTask_ActivateBossAbility::UBTTask_ActivateBossAbility()
{
	NodeName = "Activate Boss Ability";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_ActivateBossAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI || !AI->GetPawn() || !AbilityTag.IsValid()) return EBTNodeResult::Failed;

	IActionPipelineOwner* PipelineOwner = Cast<IActionPipelineOwner>(AI);
	if (!PipelineOwner) return EBTNodeResult::Failed;

	UActionPipeline* Pipeline = PipelineOwner->GetActionPipeline();
	if (!Pipeline) return EBTNodeResult::Failed;

	// ── 1. 블랙보드에서 타겟 읽기 ────────────────────────────────────────────
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	APawn* BBTarget = BB ? Cast<APawn>(BB->GetValueAsObject(TEXT("BB_CurrentTarget"))) : nullptr;

	// ── 2. Pre-GA 파이프라인 (유효성 검사) ───────────────────────────────────
	FActionData Data;
	Data.Instigator = AI->GetPawn();
	Data.Target     = BBTarget;

	if (!Pipeline->Execute(Data)) return EBTNodeResult::Failed;

	// ── 3. GA 실행 ───────────────────────────────────────────────────────
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AI->GetPawn());
	if (!ASC) return EBTNodeResult::Failed;

	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
	if (!bActivated) return EBTNodeResult::Failed;
	if (!bWaitForEnd) return EBTNodeResult::Succeeded;

	// ── 4. GA 종료 대기 (델리게이트 바인딩) ──────────────────────────────────
	TArray<FGameplayAbilitySpec*> MatchingSpecs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(AbilityTag), MatchingSpecs);

	for (FGameplayAbilitySpec* Spec : MatchingSpecs)
	{
		if (!Spec || !Spec->IsActive()) continue;

		UGameplayAbility* Instance = Spec->GetPrimaryInstance();
		if (!Instance) break;

		CachedOwnerComp = &OwnerComp;
		BoundAbility    = Instance;
		Instance->OnGameplayAbilityEnded.AddUObject(this, &UBTTask_ActivateBossAbility::HandleAbilityEnded);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Succeeded;
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
	if (BoundAbility.IsValid())
	{
		BoundAbility->OnGameplayAbilityEnded.RemoveAll(this);
	}
	BoundAbility.Reset();
}

FString UBTTask_ActivateBossAbility::GetStaticDescription() const
{
	return FString::Printf(TEXT("Activate GA: %s"), *AbilityTag.ToString());
}
