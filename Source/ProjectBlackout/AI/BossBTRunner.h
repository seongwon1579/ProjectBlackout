#pragma once

#include "CoreMinimal.h"
#include "BossPhase.h"
#include "UObject/Object.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BossBTRunner.generated.h"

class ABlackoutBossAIController;

UCLASS()
class PROJECTBLACKOUT_API UBossBTRunner : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(ABlackoutBossAIController* InController, const TMap<EBossPhase, TObjectPtr<UBehaviorTree>>& InTrees);

	void RunPhaseBT(EBossPhase NewPhase);

	void StopBT();

	EBossPhase GetCurrentPhase() const { return CurrentPhase; }

private:
	UPROPERTY(Transient)
	TObjectPtr<ABlackoutBossAIController> OwnerController;
	
	UPROPERTY(Transient)
	TMap<EBossPhase, TObjectPtr<UBehaviorTree>> PhaseBehaviorTrees;
	
	EBossPhase CurrentPhase;
};
