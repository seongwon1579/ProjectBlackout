#include "Combat/Weapons/BOChicagoTypewriter.h"

#include "GameplayTags/BlackoutGameplayTags.h"

ABOChicagoTypewriter::ABOChicagoTypewriter()
{
	// 테스트용 주무기 검증에 바로 사용할 수 있도록 슬롯 태그와 기본 스탯을 설정합니다.
	WeaponTag = BlackoutGameplayTags::Weapon_Primary;
	CachedStats.WeaponTag = WeaponTag;
	CachedStats.BaseDamage = 14.0f;
	CachedStats.FireRate = 9.0f;
	CachedStats.MagazineSize = 40;
	CachedStats.MaxReserveAmmo = 200;
	CachedStats.SplashRadius = 0.0f;

	bUseHitscan = true;
	MuzzleSocket = TEXT("MuzzleSocket");
}
