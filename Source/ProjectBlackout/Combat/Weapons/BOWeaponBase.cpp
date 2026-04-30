#include "Combat/Weapons/BOWeaponBase.h"

#include "BlackoutCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

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
	CachedStats.CrosshairType = FMath::Clamp(CachedStats.CrosshairType, 0, 5);

	WeaponIcon = CachedStats.WeaponIcon;

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
	// 장착 시점에 Owner가 캐릭터로 지정되는 것을 전제로 합니다.
	return Cast<ABlackoutCharacterBase>(GetOwner());
}

float ABOWeaponBase::GetBaseDamage() const
{
	return CachedStats.BaseDamage;
}

int32 ABOWeaponBase::GetCrosshairType() const
{
	return FMath::Clamp(CachedStats.CrosshairType, 0, 5);
}

FName ABOWeaponBase::GetEquippedSocketName() const
{
	return CachedStats.EquippedSocketName;
}

FName ABOWeaponBase::GetHolsterSocketName() const
{
	return CachedStats.HolsterSocketName;
}

bool ABOWeaponBase::AttachToOwner(FName SocketName)
{
	if (SocketName.IsNone())
	{
		return false;
	}

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* CharacterMesh = Character ? Character->GetMesh() : nullptr;
	if (!CharacterMesh || !CharacterMesh->DoesSocketExist(SocketName))
	{
		return false;
	}

	AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketName);
	return true;
}

bool ABOWeaponBase::HasLeftHandIKTarget() const
{
	return WeaponMesh && !LeftHandIKSocketName.IsNone() && WeaponMesh->DoesSocketExist(LeftHandIKSocketName);
}

FTransform ABOWeaponBase::GetLeftHandIKTransform() const
{
	if (!HasLeftHandIKTarget())
	{
		return FTransform::Identity;
	}

	return WeaponMesh->GetSocketTransform(LeftHandIKSocketName);
}
