#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpFwd.h"
#include "IWebSocket.h"
#include "BlackoutMatchmakingSubsystem.generated.h"


/**
 * 매칭 서버(Nest.js)가 반환하는 세션 정보. StartMatchmaking 응답으로 채워진다.
 */
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
	// 신규 방 생성 여부. "빈 방 생성" vs "기존 방 참여" 구분용.
	UPROPERTY(BlueprintReadOnly)
	bool bIsNewRoom = false;
};

// 로그인 응답 완료. bSuccess=false면 토큰 없이 종료.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutLoginComplete, bool,
                                            bSuccess);

// 매칭 시작 성공. 반환된 세션 정보로 로비 WebSocket 구독이 예약된다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutMatchmakingStarted,
                                            const FBlackoutSessionInfo&,
                                            SessionInfo);

// 매칭 취소 완료. bSessionDestroyed=true면 마지막 인원이 나가 세션이 파괴됨.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutMatchmakingCancelled,
                                            bool, bSessionDestroyed);

// 매칭 관련 HTTP 에러. HttpStatus=0은 네트워크 자체 실패.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlackoutMatchmakingError, int32,
                                             HttpStatus, const FString&,
                                             Message);

// 같은 세션에 다른 플레이어 합류.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBlackoutPlayerJoined,
                                               const FString&, SessionId,
                                               const TArray<FString>&, Players,
                                               int32, Count);

// 같은 세션에서 플레이어 이탈.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnBlackoutPlayerLeft,
                                              const FString&, SessionId,
                                              const FString&, PlayerName,
                                              const TArray<FString>&, Players,
                                              int32, Count);

// 정원 충족으로 게임 시작 신호. bAutoTravelOnGameStart=true면 자동 ClientTravel.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBlackoutGameStart,
                                               const FString&, SessionId,
                                               const FString&, ServerIp, int32,
                                               ServerPort);

// 세션 파괴 (전원 이탈, 타임아웃 등).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlackoutSessionCancelled,
                                            const FString&, SessionId);

// 매칭 파이프라인 실패 (데디 할당 불가, 내부 오류 등).
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlackoutMatchmakingFailed,
                                             const FString&, SessionId,
                                             const FString&, Reason);

/**
 * 매칭 서버(Nest.js) HTTP + 로비 WebSocket 클라이언트.
 * Login → StartMatchmaking → WebSocket game_start 수신 → 자동 ClientTravel 전 경로 처리.
 * UGameInstanceSubsystem 이므로 GameInstance 수명 동안 유지된다.
 */
UCLASS()
class PROJECTBLACKOUT_API
	UBlackoutMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 매칭 서버 로그인. 성공 시 AccessToken 저장 후 로비 WebSocket 자동 연결.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void Login(const FString& PlayerName, const FString& Password);

	// 자동 합방 요청. 꽉 찬 waiting 방 우선 배정.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void StartMatchmaking();

	// 매칭 취소. waiting / playing 모두 허용 (playing 중 취소는 surrender로 간주).
	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void CancelMatchmaking();

	// 지정 데디 주소로 ClientTravel. 호출 전 로비 WebSocket 자동 종료.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void TravelToGameServer(const FString& ServerIp, int32 ServerPort  , const FString& SessionId = TEXT(""));

	// 로비 WebSocket 수동 연결. Login 성공 시 자동 호출되므로 외부 호출 보통 불필요.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void ConnectLobby();

	// 로비 WebSocket 종료 + CurrentSessionId 초기화.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Matchmaking")
	void DisconnectLobby();
	
	// 로그인 상태 해제 AccessToken / CachedPlayerName 폐기 , 로비 WS 종료
	// 서버 호출 없음 ( JWT 제거 상태 ) 호출직후 UI 상태 갱신
	UFUNCTION(BlueprintCallable , Category="Blackout|Matchmaking")
	void Logout();

	UFUNCTION(BlueprintPure, Category = "Blackout|Matchmaking")
	bool IsLobbyConnected() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Matchmaking")
	bool IsLoggedIn() const { return !AccessToken.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "Blackout|Matchmaking")
	FString GetPlayerName() const { return CachedPlayerName; }

	const FString& GetAccessToken() const { return AccessToken; }

	// NetworkSettings 기본값 Initialize 에서 로드. 런타임에 BP에서 토글 가능.
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
	// Authorization: Bearer <token> 헤더 주입. AccessToken 비어있으면 skip.
	void SetAuthHeader(const FHttpRequestRef& Request) const;

	void OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response,
	                     bool bSucceeded);
	void OnStartMatchmakingResponse(FHttpRequestPtr Request,
	                                FHttpResponsePtr Response, bool bSucceeded);
	void OnCancelMatchmakingResponse(FHttpRequestPtr Request,
	                                 FHttpResponsePtr Response,
	                                 bool bSucceeded);

	// 세션 join 요청 예약. WebSocket 미연결이면 PendingSessionId 에 버퍼링 후 연결 시 flush.
	void SendJoinSessionMessage(const FString& SessionId);

	void HandleWsMessage(const FString& MessageStr);
	void HandleWsConnected();

	// PendingSessionId 있으면 즉시 join_session 송신.
	void FlushPendingJoin();

	FString AccessToken;
	FString CachedPlayerName;
	TSharedPtr<IWebSocket> WebSocket;
	// 현재 참여 중인 세션. FlushPendingJoin 시점에 확정.
	FString CurrentSessionId;
	// WebSocket 미연결 시점의 join_session 요청 대기 큐.
	FString PendingSessionId;
};
