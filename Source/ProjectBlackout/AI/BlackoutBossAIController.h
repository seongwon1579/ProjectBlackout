#pragma once

#include "CoreMinimal.h"
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

	// 데미지를 받았을때 호출
	virtual void RecordDamage(APawn* Source, float Amount);

protected:

	virtual void OnPossess  (APawn* InPawn) override;
	virtual void OnUnPossess()              override;
	
	virtual void PreInitialize(APawn* InPawn);
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	virtual void HandleAggroTargetChanged(APawn* NewTarget) {}
	
private:
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutAggroEvaluator> AggroEvaluator;
	
};
