#include "Combat/Weapons/BlackoutMeridian.h"

#include "Combat/Weapons/BOMeridianGrenadeProjectile.h"
#include "GameplayTags/BlackoutGameplayTags.h"

ABlackoutMeridian::ABlackoutMeridian()
{
	// Demolition 보조무기로 바로 테스트할 수 있도록 기본 슬롯, 스탯, 발사체를 설정합니다.
	WeaponTag = BlackoutGameplayTags::Weapon_Secondary;
	CachedStats.WeaponTag = WeaponTag;
	CachedStats.BaseDamage = 90.0f;
	CachedFirearmStats.WeaponTag = WeaponTag;
	CachedFirearmStats.BaseDamage = 90.0f;
	CachedFirearmStats.FireRate = 0.75f;
	CachedFirearmStats.bIsAutomatic = false;
	CachedFirearmStats.MagazineSize = 1;
	CachedFirearmStats.MaxReserveAmmo = 12;
	CachedFirearmStats.SplashRadius = 350.0f;

	bUseHitscan = false;
	ProjectileClass = ABOMeridianGrenadeProjectile::StaticClass();
	MuzzleSocket = TEXT("MuzzleSocket");
}
