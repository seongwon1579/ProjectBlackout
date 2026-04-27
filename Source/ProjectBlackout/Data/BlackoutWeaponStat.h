#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "BlackoutWeaponStat.generated.h"

/**
 * 모든 무기가 공유하는 기본 스탯 행 구조체.
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

	/** 장착 중인 무기를 부착할 캐릭터 메시 소켓 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon")
	FName EquippedSocketName = TEXT("WeaponSocket");

	/** 수납 중인 무기를 부착할 캐릭터 메시 소켓. 비워두면 슬롯 기본 소켓을 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon")
	FName HolsterSocketName = NAME_None;
};

/**
 * 총기 전용 스탯 행 구조체.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutFirearmStat : public FBlackoutWeaponStat
{
	GENERATED_BODY()

	/** 초당 발사 횟수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0.f))
	float FireRate = 10.f;

	/** 입력을 누르고 있을 때 FireRate 간격으로 계속 발사할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon")
	bool bIsAutomatic = false;

	/** 한 탄창에 들어가는 최대 장탄수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0))
	int32 MagazineSize = 30;

	/** 소지 가능한 최대 예비 탄약 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0))
	int32 MaxReserveAmmo = 120;

	/** 산탄/유탄류 스플래시 반경 (cm). 비적용 무기는 0. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0.f))
	float SplashRadius = 0.f;
};

/**
 * 근접 무기 전용 스탯 행 구조체.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutMeleeWeaponStat : public FBlackoutWeaponStat
{
	GENERATED_BODY()

	/** 공격 판정 스윕 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (ClampMin = 0.f))
	float SwingRadius = 50.f;
};
