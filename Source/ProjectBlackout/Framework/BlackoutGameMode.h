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
	// 매치 시작 시 URL 옵션(?SessionId=...)을 파싱해 MatchmakingSessionId에 보관한다.
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void Logout(AController* Exiting) override;

	// 자식 GameMode(Lobby/Battle)가 공통 집계 뒤 확장 로직을 붙이는 훅.
	virtual void OnPlayerJoined(APlayerController* NewPlayer) {}
	virtual void OnPlayerLeft(AController* Exiting) {}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|GameMode")
	int32 MaxPlayers = 4;

	// 매칭 API(Nest.js)가 데디 URL 옵션으로 넘긴 세션 식별자. 세션 CRUD와 1:1 매핑.
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|GameMode")
	FString MatchmakingSessionId;

	// 현재 접속 중인 플레이어 컨트롤러. 서버 전용이므로 리플리케이션 대상 아님.
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|GameMode")
	TArray<TObjectPtr<APlayerController>> ConnectedPlayers;
};
