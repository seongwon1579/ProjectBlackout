// ─── 구현 내역 ───────────────────────
//  - 김민영: 피격 시 일회성 히트 연출(혈흔·사운드)을 재생하는 GameplayCue Notify
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_HitImpact.generated.h"

/**
 * 피격 시 일회성 연출(혈흔, 히트 사운드 등)을 담당하는 GameplayCue (TDD v5 §11)
 */
UCLASS()
class PROJECTBLACKOUT_API UGCN_HitImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()
	
public:
	UGCN_HitImpact();

	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
