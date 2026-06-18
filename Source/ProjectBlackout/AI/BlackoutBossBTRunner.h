// ─── 구현 내역 ───────────────────────
//  - 조성원: 페이즈별 BehaviorTree를 보관하고 페이즈 전환 시 해당 BT로 교체 실행하는 러너
// ──────────────────────────────────────

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
