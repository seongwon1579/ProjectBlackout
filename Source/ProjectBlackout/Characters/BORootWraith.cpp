#include "Characters/BORootWraith.h"
#include "Combat/Weapons/BOProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"

ABORootWraith::ABORootWraith()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch =true;
	
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	MovementComponent->DefaultLandMovementMode =MOVE_Flying;
	MovementComponent->MaxFlySpeed = DefaultFlySpeed;
	MovementComponent->BrakingDecelerationFlying = 1024.0f;
}
