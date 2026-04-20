#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlackoutGameMode.generated.h"

UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABlackoutGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|GameMode")
	virtual void HandlePartyWipe();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|GameMode")
	int32 MaxPlayers = 4;
};
