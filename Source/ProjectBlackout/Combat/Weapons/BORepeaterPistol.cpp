#include "Combat/Weapons/BORepeaterPistol.h"

#include "GameplayTags/BlackoutGameplayTags.h"

ABORepeaterPistol::ABORepeaterPistol()
{
	// 테스트용 보조무기로 바로 사용할 수 있도록 기본 슬롯 태그와 스탯을 설정합니다.
	WeaponTag = BlackoutGameplayTags::Weapon_Secondary;
	CachedStats.WeaponTag = WeaponTag;
	CachedStats.BaseDamage = 22.0f;
	CachedFirearmStats.WeaponTag = WeaponTag;
	CachedFirearmStats.BaseDamage = 22.0f;
	CachedFirearmStats.FireRate = 2.5f;
	CachedFirearmStats.bIsAutomatic = false;
	CachedFirearmStats.MagazineSize = 6;
	CachedFirearmStats.MaxReserveAmmo = 36;
	CachedFirearmStats.SplashRadius = 0.0f;

	bUseHitscan = true;
	MuzzleSocket = TEXT("MuzzleSocket");
}
