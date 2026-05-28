#include "Characters/BORootWraith.h"
#include "Combat/Weapons/BOProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

ABORootWraith::ABORootWraith()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch =true;
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->DefaultLandMovementMode =MOVE_Flying;
	MovementComponent->MaxFlySpeed = DefaultFlySpeed;
	MovementComponent->BrakingDecelerationFlying = 1024.0f;
}

void ABORootWraith::BeginPlay()
{
	Super::BeginPlay();

	PlayTeleportEndGameplayCue();
}

void ABORootWraith::OnSpawnFromPool_Implementation()
{
	Super::OnSpawnFromPool_Implementation();

	PlayTeleportEndGameplayCue();
}

void ABORootWraith::PlayTeleportEndGameplayCue()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		if (SpawnGameplayCueTag.IsValid())
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = GetActorLocation();
			ASC->ExecuteGameplayCue(SpawnGameplayCueTag, CueParams);
		}
	}
}
