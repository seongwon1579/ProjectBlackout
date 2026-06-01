#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameplayTags.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "BlackoutWeaponStat.generated.h"

class UTexture2D;

/**
 * 표면 재질 태그별로 무기 피격 GCN을 고르는 규칙.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutSurfaceImpactCueRule
{
	GENERATED_BODY()

	/** 피격 표면 재질을 의미하는 태그입니다. 예: Surface.Flesh, Surface.Metal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayTag SurfaceTag = BlackoutGameplayTags::Surface_Default;

	/** 해당 표면에 맞았을 때 실행할 무기별 피격 GCN 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayTag ImpactCueTag = BlackoutGameplayTags::GameplayCue_Weapon_Default_Impact_Default;

	/** 같은 표면 태그가 중복 등록된 경우 높은 값이 먼저 선택됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue", meta = (ClampMin = 0))
	int32 Priority = 0;

	bool MatchesSurfaceTag(const FGameplayTag& InSurfaceTag) const
	{
		return SurfaceTag.IsValid() && SurfaceTag.MatchesTagExact(InSurfaceTag);
	}
};

/**
 * 무기별 발사, 탄 궤적, 표면 재질별 피격 GCN 태그 세트.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutWeaponCueSet
{
	GENERATED_BODY()

	/** 발사 순간 총구 화염/사운드 등에 사용할 GCN 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayTag FireCueTag = BlackoutGameplayTags::GameplayCue_Weapon_Default_Fire;

	/** 탄 궤적 트레이서나 빔에 사용할 GCN 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayTag TrailCueTag = BlackoutGameplayTags::GameplayCue_Weapon_Default_Trail;

	/** 표면 규칙이 없거나 GCN 매핑이 비어 있을 때 사용할 기본 피격 GCN 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayTag DefaultImpactCueTag = BlackoutGameplayTags::GameplayCue_Weapon_Default_Impact_Default;

	/** 표면 재질 태그별 피격 GCN 선택 규칙입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	TArray<FBlackoutSurfaceImpactCueRule> SurfaceImpactRules;

	bool HasFireCue() const
	{
		return FireCueTag.IsValid();
	}

	bool HasTrailCue() const
	{
		return TrailCueTag.IsValid();
	}

	FGameplayTag ResolveImpactCue(const FGameplayTag& SurfaceTag) const
	{
		const FBlackoutSurfaceImpactCueRule* BestRule = nullptr;
		for (const FBlackoutSurfaceImpactCueRule& Rule : SurfaceImpactRules)
		{
			if (!Rule.MatchesSurfaceTag(SurfaceTag) || !Rule.ImpactCueTag.IsValid())
			{
				continue;
			}

			if (!BestRule || Rule.Priority > BestRule->Priority)
			{
				BestRule = &Rule;
			}
		}

		return BestRule ? BestRule->ImpactCueTag : DefaultImpactCueTag;
	}
};

/**
 * 모든 무기가 공유하는 기본 스탯 행 구조체.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutWeaponStat : public FTableRowBase
{
	GENERATED_BODY()

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

	/** 무기별 발사/탄 궤적/표면 재질별 피격 GCN 태그 세트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Cue")
	FBlackoutWeaponCueSet CueSet;
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

	/** AnimBP에서 양손 총기 애니메이션 세트를 사용할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bUseTwoHandedAnimation = true;

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

	/** 반동으로 인한 카메라 수직 상승 최대 각도 (도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Recoil", meta = (ClampMin = 0.f))
	float MaxRecoilPitchDegrees = 15.0f;

	/** 반동 종료 후 자동으로 되돌아오는 비율 (0=되돌림 없음, 1=전부 되돌림). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Recoil", meta = (ClampMin = 0.f, ClampMax = 1.f))
	float RecoilRecoveryFraction = 0.4f;
};

/**
 * 산탄 총기 전용 스탯 행 구조체.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutShotgunFirearmStat : public FBlackoutFirearmStat
{
	GENERATED_BODY()

	/** 한 번의 사격에서 생성되는 펠릿 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Shotgun", meta = (ClampMin = 1))
	int32 PelletCount = 8;

	/** 펠릿이 퍼지는 원뿔 각도 (도) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Shotgun", meta = (ClampMin = 0.f))
	float PelletSpreadDegrees = 6.0f;

	/** 펠릿별 라인트레이스 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Shotgun", meta = (ClampMin = 0.f))
	float PelletTraceDistance = 5000.0f;

	/** 펠릿 한 발의 기본 피해량. 0 이하이면 BaseDamage / PelletCount를 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Shotgun", meta = (ClampMin = 0.f))
	float DamagePerPellet = 0.0f;

	/** 단일 타겟에 적용되는 펠릿 수를 제한할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Shotgun")
	bool bSingleTargetPelletCap = false;

	/** 단일 타겟에 피해를 적용할 최대 펠릿 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Shotgun", meta = (ClampMin = 1, EditCondition = "bSingleTargetPelletCap"))
	int32 MaxPelletsPerTarget = 4;
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
