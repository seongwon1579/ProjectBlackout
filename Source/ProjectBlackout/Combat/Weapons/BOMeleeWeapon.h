// ─── 구현 내역 ───────────────────────
//  - 김민영: 근접 무기 클래스(캡슐 스윕 판정, 근접 스탯 데이터테이블 초기화) 구현
//  - 허혁: 근접 무기 데미지 전달 연동
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "BOMeleeWeapon.generated.h"

class UBoxComponent;

UCLASS()
class PROJECTBLACKOUT_API ABOMeleeWeapon : public ABOWeaponBase
{
	GENERATED_BODY()
	
public:
	ABOMeleeWeapon();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	TArray<FHitResult> PerformSweep(const FVector& Forward);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void SetHitBoxActive(bool bActive);

	virtual bool InitializeStatsFromDataTable() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UBoxComponent> HitBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (RowType = "/Script/ProjectBlackout.BlackoutMeleeWeaponStat"))
	FDataTableRowHandle MeleeStatsRow;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Weapon")
	FBlackoutMeleeWeaponStat CachedMeleeStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float SwingRadius = 50.0f;
};
