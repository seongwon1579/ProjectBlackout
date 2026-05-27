#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOFirearm.h"
#include "BOShotgunFirearm.generated.h"

/**
 * 산탄 펠릿 1개의 처리 결과.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutShotgunPelletHit
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Combat")
	int32 PelletIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Combat")
	FHitResult HitResult;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Combat")
	FGameplayTag HitPartTag;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Combat")
	float DamageApplied = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Combat")
	bool bAppliedDamage = false;
};

/**
 * 포자피개, 더블 배럴처럼 다중 펠릿 히트스캔을 사용하는 총기 베이스 클래스.
 */
UCLASS(HideCategories = ("Blackout|Weapon"))
class PROJECTBLACKOUT_API ABOShotgunFirearm : public ABOFirearm
{
	GENERATED_BODY()

public:
	ABOShotgunFirearm();

	virtual bool InitializeStatsFromDataTable() override;

	virtual FDataTableRowHandle GetUIStatsRow() const override { return ShotgunStatsRow; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	TArray<FBlackoutShotgunPelletHit> FireShotgun(const FVector& BaseDirection, const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	TArray<FVector> BuildPelletDirections(const FVector& BaseDirection) const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetPelletCount() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetPelletSpreadDegrees() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetPelletTraceDistance() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetDamagePerPellet() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool ShouldApplySingleTargetPelletCap() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMaxPelletsPerTarget() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Shotgun", meta = (RowType = "/Script/ProjectBlackout.BlackoutShotgunFirearmStat"))
	FDataTableRowHandle ShotgunStatsRow;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Shotgun")
	FBlackoutShotgunFirearmStat CachedShotgunStats;
};
