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
	virtual void StartBattle();

protected:
	virtual void OnPlayerJoined(APlayerController* NewPlayer) override;

	// 전원 Ready 성립 시 StartBattle 트리거.
	virtual void OnAllPlayersReady() override;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Lobby")
	TArray<FSoftObjectPath> BossStageMapPaths;
	
	// ServerTravel 중복 실행 방지 플래그 , StartBattle 최초 1회만
	UPROPERTY(BlueprintReadOnly , Category = "Blackout|Lobby")
	bool bTravelInitiated = false; 
};
