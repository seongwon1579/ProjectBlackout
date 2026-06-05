#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_FireExplosiveArrow.h"

#include "BOProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

void UBlackoutGA_Shrewd_FireExplosiveArrow::LaunchProjectile(ABOProjectile* Arrow, const FVector& SpawnLocation,
                                                             const FVector& TargetLocation)
{
	if (!Arrow) return;
	UWorld* World = Arrow->GetWorld();
	if (!World) return;

	const float N = SpeedMultiplier;
	if (UProjectileMovementComponent* PMC = Arrow->FindComponentByClass<UProjectileMovementComponent>())
	{
		PMC->ProjectileGravityScale = N * N;
	}

	const float EffectiveGravityZ = World->GetGravityZ() * N * N;

	FVector LaunchVelocity;
	const bool bCalcSuccess = UGameplayStatics::SuggestProjectileVelocity_CustomArc(
		World, 
		LaunchVelocity, 
		SpawnLocation, 
		TargetLocation,
		EffectiveGravityZ,
		ArcParam);
	
	if (bCalcSuccess)
	{
		Arrow->Launch(LaunchVelocity);
	}
	else
	{
		const FVector Fallback = (TargetLocation - SpawnLocation).GetSafeNormal() * FallbackStraightSpeed;
		Arrow->Launch(Fallback);
	}
}
