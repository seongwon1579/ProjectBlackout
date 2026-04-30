#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "BlackoutWeaponStat.generated.h"

class UTexture2D;

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

	/** HUD에 표시할 무기 아이콘. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|UI")
	TObjectPtr<UTexture2D> WeaponIcon = nullptr;

	/** 조준 시 HUD에 표시할 크로스헤어 종류. 외부 크로스헤어 에셋의 0~5 타입과 대응합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|UI", meta = (ClampMin = 0, ClampMax = 5, UIMin = 0, UIMax = 5))
	int32 CrosshairType = 0;

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

	/** 발사하지 않을 때의 기본 탄퍼짐 각도 (도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Spread", meta = (ClampMin = 0.f))
	float BaseSpreadDegrees = 0.5f;

	/** 연사 시 도달할 수 있는 최대 탄퍼짐 각도 (도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Spread", meta = (ClampMin = 0.f))
	float MaxSpreadDegrees = 5.0f;

	/** 발사 1회당 증가하는 탄퍼짐 각도 (도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Spread", meta = (ClampMin = 0.f))
	float SpreadPerShot = 0.8f;

	/** 초당 탄퍼짐 회복량 (도/초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Spread", meta = (ClampMin = 0.f))
	float SpreadRecoveryRate = 4.0f;

	/** 발사 1회당 적용되는 수직 반동 최솟값 (도). 카메라가 위로 올라갑니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Recoil", meta = (ClampMin = 0.f))
	float VerticalRecoilMin = 0.5f;

	/** 발사 1회당 적용되는 수직 반동 최댓값 (도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Recoil", meta = (ClampMin = 0.f))
	float VerticalRecoilMax = 1.2f;

	/** 발사 1회당 적용되는 수평 반동의 최대 절댓값 (도). 이 값의 ±범위 안에서 무작위 적용됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Recoil", meta = (ClampMin = 0.f))
	float HorizontalRecoilRange = 0.4f;
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
