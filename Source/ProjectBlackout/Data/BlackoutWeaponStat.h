#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "BlackoutWeaponStat.generated.h"

/**
 * DT_WeaponStats DataTable 행 구조체.
 * 무기별 기본 스탯. UBlackoutCombatComponent에서 무기 스탯 조회에 사용.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutWeaponStat : public FTableRowBase
{
	GENERATED_BODY()

	/** 무기 식별 태그 (e.g. Weapon.Primary.Rifle) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0.f))
	float BaseDamage = 20.f;

	/** 초당 발사 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0.f))
	float FireRate = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0))
	int32 MagazineSize = 30;

	/** 산탄/유탄류 스플래시 반경 (cm). 비적용 무기는 0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0.f))
	float SplashRadius = 0.f;
};
