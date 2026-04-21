#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "UObject/SoftObjectPath.h"
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
	
	UFUNCTION(BlueprintCallable, Category = "Blackout|Lobby")
	virtual void NotifyReadyChanged();
	
protected:
	virtual void OnPlayerJoined(APlayerController* NewPlayer) override;
	
	UPROPERTY(EditDefaultsOnly  , Category = "Blackout|Lobby")
	FSoftObjectPath BattleMapPath;
};
