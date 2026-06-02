#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_TeleportToPoint.h"
#include "AbilitySystemComponent.h"

#include "BOShrewdBoss.h"

void UBlackoutGA_Shrewd_TeleportToPoint::StartResolveDestination()
{
	FVector OutLocation; 
	FRotator OutRotation;
	
	bool bIsSuccess = ResolveTeleportDestination(OutLocation, OutRotation);
	
	CachedDestination = OutLocation;
	CachedTeleportRotation = OutRotation;
	
	FinishPrepare (bIsSuccess);
}

bool UBlackoutGA_Shrewd_TeleportToPoint::ResolveTeleportDestination(FVector& OutLocation, FRotator& OutRotation)
{
	ABOShrewdBoss* Shrewd = Cast<ABOShrewdBoss>(GetAvatarActorFromActorInfo());
	if (!Shrewd) return false;

	FTransform Transform;
	if (!Shrewd->GetRandomTeleportTransform(Transform)) return false;

	OutLocation = Transform.GetLocation();
	OutRotation = Transform.Rotator();
	return true;
}
