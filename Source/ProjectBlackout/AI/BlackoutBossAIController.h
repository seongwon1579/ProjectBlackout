#pragma once

#include "CoreMinimal.h"
#include "BossPhase.h"
#include "GameplayTagContainer.h"
#include "AI/BlackoutAIController.h"
#include "AI/ActionPipelineOwner.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BlackoutBossAIController.generated.h"

class UBossBTRunner;
class UAbilitySystemComponent;
struct FGameplayTag;
class UBehaviorTree;
class UActionPipeline;

/**
 * 보스 전용 AI 컨트롤러.
 * 에디터에서 BehaviorTreeAsset을 지정하면 OnPossess 시 자동 실행된다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutBossAIController : public ABlackoutAIController
{
	GENERATED_BODY()

public:
	ABlackoutBossAIController();
	
	// 외부(체력 이벤트 등)에서 페이즈 전환 요청
	UFUNCTION()
	void RequestPhaseChange(EBossPhase NewPhase);
	
protected:

	virtual void OnPossess  (APawn* InPawn) override;
	virtual void OnUnPossess()              override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|BT")
	TMap<EBossPhase, TObjectPtr<UBehaviorTree>> PhaseBehaviorTrees;


private:
	
	// 락 태그가 변할 때 호출되는 콜백
	void OnPhaseLockTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	void TryApplyPendingPhase();
	
	void ApplyPhaseChange(EBossPhase NewPhase);
	
	bool IsPhaseTransitionLocked() const;
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;
	
	UPROPERTY(Transient)
	TObjectPtr<UBossBTRunner> BTRunner;
	
	UPROPERTY(Transient)
	
	EBossPhase PendingPhase = EBossPhase::None;
	
	EBossPhase CurrentPhase = EBossPhase::None;
	
	FDelegateHandle PhaseLockTagChangedHandle;
	
	FGameplayTag PhaseLockTag;
	
	UFUNCTION()
	void CycleTarget();

	UPROPERTY()
	TObjectPtr<UActionPipeline> ActionPipeline;

	FTimerHandle TargetCycleTimerHandle;
	int32        CurrentTargetIndex = 0;
};
