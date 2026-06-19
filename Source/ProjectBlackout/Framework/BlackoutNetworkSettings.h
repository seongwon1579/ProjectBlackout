

// ─── 구현 내역 ───────────────────────
//  - 최승현: 매칭 네트워크 설정 — API/WS URL·자동 트래블·더미 패스워드 프로젝트 설정
// ──────────────────────────────────────

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
	
	// 실 값은 Project Settings > Blackout > Network 또는 DefaultGame.ini 에서 입력.
	UPROPERTY(Config, EditAnywhere , BlueprintReadOnly , Category="Matchmaking")
	FString DummyPassword;
	
	// Project Settings UI 카테고리 Blackout
	virtual FName GetCategoryName() const override {return TEXT("Blackout");}
};
