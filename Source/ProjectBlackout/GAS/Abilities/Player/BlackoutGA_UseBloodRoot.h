// ─── 구현 내역 ───────────────────────
//  - 김민영: 블러드 루트 소모품 전용 GA 분리, 취소 시 쿨다운 초기화
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Player/BlackoutGA_UseConsumable.h"
#include "BlackoutGA_UseBloodRoot.generated.h"

class UBOConsumableData;

/**
 * 블러드 루트 사용 어빌리티.
 * 공통 소모품 사용 절차 이후 ASC 지속 체력 회복을 적용합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API UBlackoutGA_UseBloodRoot : public UBlackoutGA_UseConsumable
{
	GENERATED_BODY()

protected:
	/** 블러드 루트 지속 회복 틱 간격입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Consumable", meta = (ClampMin = 0.1))
	float HealTickInterval = 1.0f;

	virtual bool ApplyConsumableEffect(const UBOConsumableData* UsedConsumableData) override;
	virtual bool ShouldApplyConfiguredGameplayEffect(const UBOConsumableData* UsedConsumableData) const override;
};
