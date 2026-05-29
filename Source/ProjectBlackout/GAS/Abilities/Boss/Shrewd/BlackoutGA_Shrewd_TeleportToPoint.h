#pragma once

#include "CoreMinimal.h"
#include "BlackoutGA_Shrewd_TeleportBase.h"
#include "BlackoutGA_Shrewd_TeleportToPoint.generated.h"

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_TeleportToPoint : public UBlackoutGA_Shrewd_TeleportBase
{
	GENERATED_BODY()

protected:
	virtual void StartResolveDestination() override;
	
	bool ResolveTeleportDestination(FVector& OutLocation, FRotator& OutRotation);
};
