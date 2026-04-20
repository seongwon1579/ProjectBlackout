#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "BlackoutLobbyGameMode.generated.h"

UCLASS()
class PROJECTBLACKOUT_API ABlackoutLobbyGameMode : public ABlackoutGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|Lobby")
	virtual bool AllPlayersReady() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Lobby")
	virtual void StartBattle();
};
