#include "Combat/Weapons/BOMeleeWeapon.h"
#include "Components/BoxComponent.h"

ABOMeleeWeapon::ABOMeleeWeapon()
{
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetupAttachment(WeaponMesh);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 스윕(공격 판정) 중에만 활성화됨
}

TArray<FHitResult> ABOMeleeWeapon::PerformSweep(const FVector& Forward)
{
	TArray<FHitResult> HitResults;
	// TODO: Implement Box Sweep based on HitBox location and Forward vector
	return HitResults;
}

void ABOMeleeWeapon::SetHitBoxActive(bool bActive)
{
	if (HitBox)
	{
		HitBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}
