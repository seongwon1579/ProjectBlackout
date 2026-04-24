#include "AI/BTTask_ActivateBossAbility.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbility.h"

UBTTask_ActivateBossAbility::UBTTask_ActivateBossAbility()
{
	NodeName = "Activate Boss Ability";
	bCreateNodeInstance = true; // 멤버 변수로 상태 보관하기 위해 인스턴스화
}

EBTNodeResult::Type UBTTask_ActivateBossAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI || !AI->GetPawn() || !AbilityTag.IsValid()) return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AI->GetPawn());
	if (!ASC) return EBTNodeResult::Failed;

	const bool bActivated = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
	if (!bActivated) return EBTNodeResult::Failed;
	if (!bWaitForEnd) return EBTNodeResult::Succeeded;

	// 활성화된 어빌리티 인스턴스를 찾아 종료 델리게이트 바인딩
	TArray<FGameplayAbilitySpec*> MatchingSpecs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(AbilityTag), MatchingSpecs);

	for (FGameplayAbilitySpec* Spec : MatchingSpecs)
	{
		if (!Spec || !Spec->IsActive()) continue;

		UGameplayAbility* Instance = Spec->GetPrimaryInstance();
		if (!Instance) break; // Non-instanced ability: 아래에서 Succeeded 반환

		CachedOwnerComp = &OwnerComp;
		BoundAbility    = Instance;
		Instance->OnGameplayAbilityEnded.AddUObject(this, &UBTTask_ActivateBossAbility::HandleAbilityEnded);
		return EBTNodeResult::InProgress;
	}

	// Non-instanced ability이거나 이미 종료된 경우 즉시 완료
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
	// 어빌리티는 취소하지 않는다 — GA가 끝까지 실행되도록 허용
	// 다음 BT 노드 실행만 막으면 되므로 델리게이트만 해제한다
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