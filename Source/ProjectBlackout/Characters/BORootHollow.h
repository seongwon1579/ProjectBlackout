// ─── 구현 내역 ───────────────────────
//  - 김민영: Root Hollow 일반 미니언 클래스 — 순수 StateTree 기반 추격·돌진 행동
//  - 조성원: 미니언 모션 워핑 컴포넌트 추가
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutMinionCharacter.h"
#include "BORootHollow.generated.h"

class UMotionWarpingComponent;
class UStateTree;

/**
 * Root Hollow 일반 미니언.
 * 순수 StateTree를 이용해 추격·돌진 행동을 수행.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORootHollow : public ABlackoutMinionCharacter
{
	GENERATED_BODY()

public:
	ABORootHollow();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
};
