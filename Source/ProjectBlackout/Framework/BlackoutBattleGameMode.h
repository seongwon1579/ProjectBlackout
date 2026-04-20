#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "BlackoutBattleGameMode.generated.h"

UCLASS()
class PROJECTBLACKOUT_API ABlackoutBattleGameMode : public ABlackoutGameMode
{
	GENERATED_BODY()

public:
	virtual void HandlePartyWipe() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	virtual void HandleCheckpoint();
};
