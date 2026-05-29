#pragma once

#include "CoreMinimal.h"
#include "Enum/BOBossPhase.h"
#include "AI/BlackoutAIController.h"
#include "BlackoutBossAIController.generated.h"

class UBlackoutAggroEvaluator;
class UBlackoutPhaseEvaluator;
class UBlackoutBossBTRunner;
class UAbilitySystemComponent;

UCLASS()
class PROJECTBLACKOUT_API ABlackoutBossAIController : public ABlackoutAIController
{
	GENERATED_BODY()

public:
	// 외부(체력 이벤트 등)에서 페이즈 전환 요청
	UFUNCTION()
	void RequestPhaseChange(EBOBossPhase NewPhase);
	
	// 데미지를 받았을때 호출
	void RecordDamage(APawn* Source, float Amount);

	// 현재 보스의 페이즈 정보를 반환
	EBOBossPhase GetCurrentPhase() const;
	
protected:

	virtual void OnPossess  (APawn* InPawn) override;
	virtual void OnUnPossess()              override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|BT")
	TMap<EBOBossPhase, TObjectPtr<UBehaviorTree>> PhaseBehaviorTrees;


private:

	void HandlePhaseChanged(EBOBossPhase NewPhase);
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;
	
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutBossBTRunner> BTRunner;
	
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutPhaseEvaluator> PhaseEvaluator;
	
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutAggroEvaluator> AggroEvaluator;
	
};
