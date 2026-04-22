#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpFwd.h"
#include "IWebSocket.h"
#include "BlackoutMatchmakingSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FBlackoutSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SessionId;
	UPROPERTY(BlueprintReadOnly)
	FString Status;
	UPROPERTY(BlueprintReadOnly)
	TArray<FString> Players;
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 0;
	UPROPERTY(BlueprintReadOnly)
	FString ServerId;
	UPROPERTY(BlueprintReadOnly)
	FString ServerIp;
	UPROPERTY(BlueprintReadOnly)
	int32 ServerPort = 0;
	UPROPERTY(BlueprintReadOnly)
	bool bIsNewRoom = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutLoginComplete, bool,
                                            bSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutMatchmakingStarted,
                                            const FBlackoutSessionInfo&,
                                            SessionInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutMatchmakingCancelled,
                                            bool, bSessionDestroyed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlackoutMatchmakingError, int32,
                                             HttpStatus, const FString&,
                                             Message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBlackoutPlayerJoined,
                                               const FString&, SessionId,
                                               const TArray<FString>&, Players,
                                               int32, Count);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnBlackoutPlayerLeft,
                                              const FString&, SessionId,
                                              const FString&, PlayerName,
                                              const TArray<FString>&, Players,
                                              int32, Count);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBlackoutGameStart,
                                               const FString&, SessionId,
                                               const FString&, ServerIp, int32,
                                               ServerPort);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutSessionCancelled,
                                            const FString&, SessionId);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlackoutMatchmakingFailed,
                                             const FString&, SessionId,
                                             const FString&, Reason);

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API
	UBlackoutMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void Login(const FString& PlayerName, const FString& Password);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void StartMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void CancelMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void TravelToGameServer(const FString& ServerIp, int32 ServerPort);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void ConnectLobby();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void DisconnectLobby();

	UFUNCTION(BlueprintPure, Category = "Blackout|Matchmaking")
	bool IsLobbyConnected() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Matchmaking")
	bool IsLoggedIn() const { return !AccessToken.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "Blackout|Matchmaking")
	FString GetPlayerName() const { return CachedPlayerName; }

	const FString& GetAccessToken() const { return AccessToken; }

	// NetworkSettings 기본값 Initialize 에서 로드
	UPROPERTY(BlueprintReadWrite, Category = "Blackout|Matchmaking")
	bool bAutoTravelOnGameStart = true;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutLoginComplete OnLoginComplete;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutMatchmakingStarted OnMatchmakingStarted;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutMatchmakingCancelled OnMatchmakingCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutMatchmakingError OnMatchmakingError;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutGameStart OnGameStart;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutSessionCancelled OnSessionCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Matchmaking")
	FOnBlackoutMatchmakingFailed OnMatchmakingFailed;

private:
	void SetAuthHeader(const FHttpRequestRef& Request) const;
	void OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response,
	                     bool bSucceeded);
	void OnStartMatchmakingResponse(FHttpRequestPtr Request,
	                                FHttpResponsePtr Response, bool bSucceeded);
	void OnCancelMatchmakingResponse(FHttpRequestPtr Request,
	                                 FHttpResponsePtr Response,
	                                 bool bSucceeded);
	void SendJoinSessionMessage(const FString& SessionId);
	void HandleWsMessage(const FString& MessageStr);
	void HandleWsConnected();
	void FlushPendingJoin();

	FString AccessToken;
	FString CachedPlayerName;
	TSharedPtr<IWebSocket> WebSocket;
	FString CurrentSessionId;
	FString PendingSessionId;
};
