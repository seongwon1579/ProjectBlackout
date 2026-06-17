// ─── 구현 내역 ───────────────────────
//  - 조성원: 보스 공격 판정 구간을 정의하고 시작/종료 시 ASC에 충돌 GameplayEvent를 발송
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_BossAttackSweep.generated.h"

/**
 * 보스 공격 판정 구간을 정의하는 AnimNotifyState.
 * Begin 시 Event.Boss.Attack.SweepStart, End 시 Event.Boss.Attack.SweepEnd를 ASC에 전달한다.
 */
UCLASS()
class PROJECTBLACKOUT_API UAnimNotifyState_BossAttackSweep : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
