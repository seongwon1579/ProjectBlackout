#pragma once

#include "CoreMinimal.h"
#include "Enum/BOBossPhase.h"
#include "UObject/Object.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BlackoutBossBTRunner.generated.h"

class ABlackoutBossAIController;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutBossBTRunner : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(ABlackoutBossAIController* InController, const TMap<EBOBossPhase, TObjectPtr<UBehaviorTree>>& InTrees);

	void RunPhaseBT(EBOBossPhase NewPhase);

	void StopBT();

	EBOBossPhase GetCurrentPhase() const { return CurrentPhase; }

private:
	UPROPERTY(Transient)
	TObjectPtr<ABlackoutBossAIController> OwnerController;
	
	UPROPERTY(Transient)
	TMap<EBOBossPhase, TObjectPtr<UBehaviorTree>> PhaseBehaviorTrees;
	
	EBOBossPhase CurrentPhase;
};
