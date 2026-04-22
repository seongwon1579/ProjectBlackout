

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "BlackoutNetworkSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game ,DefaultConfig , meta= (DisplayName ="Blackout Network"))
class PROJECTBLACKOUT_API UBlackoutNetworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config , EditAnywhere , BlueprintReadOnly , Category="Matchmaking")
	FString ApiBaseUrl = TEXT("http://localhost:3000");
	
	UPROPERTY(Config , EditAnywhere , BlueprintReadOnly , Category="Matchmaking")
	FString LobbyWsUrl = TEXT("ws://localhost:3001");
	
	UPROPERTY(Config , EditAnywhere , BlueprintReadOnly , Category="Matchmaking")
	bool bAutoTravelOnGameStart = true;
	
	// Project Settings UI 카테고리 Blackout
	virtual FName GetCategoryName() const override {return TEXT("Blackout");}
};
