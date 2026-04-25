#include "Combat/Weapons/BOWeaponBase.h"

#include "BlackoutCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h" // Using Character as base for now

ABOWeaponBase::ABOWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void ABOWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeStatsFromDataTable();
}

bool ABOWeaponBase::InitializeStatsFromDataTable()
{
	return false;
}

void ABOWeaponBase::ApplyCommonStats(const FBlackoutWeaponStat& WeaponStats)
{
	CachedStats = WeaponStats;

	if (CachedStats.WeaponTag.IsValid())
	{
		WeaponTag = CachedStats.WeaponTag;
	}
	else
	{
		CachedStats.WeaponTag = WeaponTag;
	}
}

ABlackoutCharacterBase* ABOWeaponBase::GetOwningCharacter() const
{
	// Assuming owner is set when equipping
	return Cast<ABlackoutCharacterBase>(GetOwner());
}

float ABOWeaponBase::GetBaseDamage() const
{
	return CachedStats.BaseDamage;
}

void ABOWeaponBase::AttachToOwner(FName SocketName)
{
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	}
}
