// ─── 구현 내역 ───────────────────────
//  - 조성원: 보스 페이즈 전환을 평가/적용하고 잠금 태그 동안 전환을 보류했다 해제 시 반영하는 평가기
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "Enum/BOBossPhase.h"
#include "BlackoutPhaseEvaluator.generated.h"

class AAIController;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, EBOBossPhase);

UCLASS()
class PROJECTBLACKOUT_API UBlackoutPhaseEvaluator : public UObject
{
	GENERATED_BODY()

public:
	UBlackoutPhaseEvaluator();
	virtual UWorld* GetWorld() const override;

	void Initialize(AAIController* InAIController, UAbilitySystemComponent* InASC);
	void Deinitialize();

	void RequestPhaseChange(EBOBossPhase NewPhase);
	bool IsPhaseTransitionLocked() const;

	EBOBossPhase GetCurrentPhase() const { return CurrentPhase; }

	FOnBossPhaseChanged OnBossPhaseChanged;

private:
	void OnPhaseLockTagChanged(const FGameplayTag Tag, int32 NewCount);
	void TryApplyPendingPhase();
	void ApplyPhaseChange(EBOBossPhase NewPhase);

	UPROPERTY(Transient)
	TObjectPtr<AAIController> CachedOwnerAIController;

	UPROPERTY(Transient)
	UAbilitySystemComponent* CachedASC;

	EBOBossPhase CurrentPhase = EBOBossPhase::None;
	EBOBossPhase PendingPhase = EBOBossPhase::None;

	FGameplayTag PhaseLockTag;
	FDelegateHandle PhaseLockTagChangedHandle;
};