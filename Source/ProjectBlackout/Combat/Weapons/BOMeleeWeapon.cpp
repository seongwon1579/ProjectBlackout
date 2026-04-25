#include "Combat/Weapons/BOMeleeWeapon.h"

#include "Components/BoxComponent.h"
#include "Engine/World.h"

ABOMeleeWeapon::ABOMeleeWeapon()
{
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetupAttachment(WeaponMesh);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 스윕(공격 판정) 중에만 활성화됨
}

bool ABOMeleeWeapon::InitializeStatsFromDataTable()
{
	Super::InitializeStatsFromDataTable();

	if (const FBlackoutMeleeWeaponStat* FoundStats = MeleeStatsRow.GetRow<FBlackoutMeleeWeaponStat>(TEXT("BOMeleeWeapon::InitializeStatsFromDataTable")))
	{
		CachedMeleeStats = *FoundStats;
		ApplyCommonStats(CachedMeleeStats);
		SwingRadius = CachedMeleeStats.SwingRadius;
		return true;
	}

	return false;
}

TArray<FHitResult> ABOMeleeWeapon::PerformSweep(const FVector& Forward)
{
	TArray<FHitResult> HitResults;

	if (!HitBox || !GetWorld())
	{
		return HitResults;
	}

	const FVector SweepStart = HitBox->GetComponentLocation();
	const FVector SweepEnd = SweepStart + Forward.GetSafeNormal() * SwingRadius;
	const FQuat SweepRotation = HitBox->GetComponentQuat();
	const FCollisionShape CollisionShape = FCollisionShape::MakeBox(HitBox->GetScaledBoxExtent());

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BOMeleeWeapon_Sweep), false, GetOwner());
	QueryParams.AddIgnoredActor(this);

	GetWorld()->SweepMultiByChannel(HitResults, SweepStart, SweepEnd, SweepRotation, ECC_Pawn, CollisionShape, QueryParams);
	return HitResults;
}

void ABOMeleeWeapon::SetHitBoxActive(bool bActive)
{
	if (HitBox)
	{
		HitBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}
