#include "Combat/Weapons/BOSteelSword.h"

ABOSteelSword::ABOSteelSword()
{
	// 테스트 단계에서 체감 가능한 기본 근접 피해와 스윕 범위를 사용합니다.
	CachedStats.BaseDamage = 35.0f;
	CachedStats.FireRate = 1.0f;
	CachedStats.MagazineSize = 0;
	CachedStats.MaxReserveAmmo = 0;
	CachedStats.SplashRadius = 0.0f;

	SwingRadius = 110.0f;
}
