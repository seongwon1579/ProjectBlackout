#include "Combat/Weapons/BOWeaponBase.h"

#include "BlackoutCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h" // Using Character as base for now

ABOWeaponBase::ABOWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;
	
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void ABOWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	// TODO: WeaponTag를 기반으로 DT_WeaponStats에서 CachedStats를 가져와야 함
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
