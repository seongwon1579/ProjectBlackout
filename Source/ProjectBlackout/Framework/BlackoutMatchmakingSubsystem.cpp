#include "BlackoutMatchmakingSubsystem.h"
#include "BlackoutNetworkSettings.h"
#include "BlackoutLog.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "WebSocketsModule.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

#include "MoviePlayer.h"
#include "UI/SBlackoutLoadingScreen.h"

// GameInstance 부팅 시 호출. WebSockets 모듈 로드 + NetworkSettings 기본값 적용.
void UBlackoutMatchmakingSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FModuleManager::Get().LoadModuleChecked(TEXT("WebSockets"));

	if (const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>())
	{
		bAutoTravelOnGameStart = Settings->bAutoTravelOnGameStart;
		BO_LOG_NET(Log, "Matchmaking Subsystem 초기화 - API=%s WS=%s",*Settings->ApiBaseUrl,*Settings->LobbyWsUrl);
	}
}

// GameInstance 종료 시 WebSocket 정리 + 토큰 폐기.
void UBlackoutMatchmakingSubsystem::Deinitialize()
{
	DisconnectLobby();
	AccessToken.Empty();
	CachedPlayerName.Empty();
	Super::Deinitialize();
}

// POST /auth/login — JSON body로 계정 전송. 응답은 OnLoginResponse.
void UBlackoutMatchmakingSubsystem::Login(const FString& PlayerName, const FString& Password)
{
	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	const FString BodyStr = FString::Printf(
		TEXT("{\"playerName\":\"%s\",\"password\":\"%s\"}"),
		*PlayerName, *Password);

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->ApiBaseUrl + TEXT("/auth/login"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(BodyStr);
	Request->OnProcessRequestComplete().BindUObject(this, &UBlackoutMatchmakingSubsystem::OnLoginResponse);

	CachedPlayerName = PlayerName;
	Request->ProcessRequest();

	BO_LOG_NET(Log, "Login 요청 전송: %s", *PlayerName);
}

// POST /matchmaking/start — Bearer 토큰 필요. 응답은 OnStartMatchmakingResponse.
void UBlackoutMatchmakingSubsystem::StartMatchmaking()
{
	if (!IsLoggedIn())
	{
		BO_LOG_NET(Warning, "로그인 필요");
		OnMatchmakingError.Broadcast(401, TEXT("로그인 필요"));
		return;
	}

	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->ApiBaseUrl + TEXT("/matchmaking/start"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	SetAuthHeader(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UBlackoutMatchmakingSubsystem::OnStartMatchmakingResponse);
	Request->ProcessRequest();

	BO_LOG_NET(Log, "StartMatchmaking 요청");
}

// DELETE /matchmaking — 세션 이탈 (playing 중 허용: surrender).
void UBlackoutMatchmakingSubsystem::CancelMatchmaking()
{
	if (!IsLoggedIn())
	{
		OnMatchmakingError.Broadcast(401, TEXT("로그인 필요"));
		return;
	}

	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->ApiBaseUrl + TEXT("/matchmaking"));
	Request->SetVerb(TEXT("DELETE"));
	SetAuthHeader(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UBlackoutMatchmakingSubsystem::OnCancelMatchmakingResponse);
	Request->ProcessRequest();

	BO_LOG_NET(Log, "CancelMatchmaking 요청");
}

// 데디 IP:Port 로 ClientTravel. 호출 전 로비 WebSocket 정리.
void UBlackoutMatchmakingSubsystem::TravelToGameServer(const FString& ServerIp, int32 ServerPort ,const FString& SessionId)
{
	if (ServerIp.IsEmpty() || ServerPort <= 0)
	{
		BO_LOG_NET(Error, "잘못된 데디 주소 - %s:%d", *ServerIp, ServerPort);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		BO_LOG_NET(Error, "World 없음 - travel 불가");
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		BO_LOG_NET(Error, "PlayerController 없음 - travel 불가");
		return;
	}
	
	FString Addr = FString::Printf(TEXT("%s:%d"), *ServerIp, ServerPort);
	if (!SessionId.IsEmpty())
	{
		Addr += FString::Printf(TEXT("?SessionId=%s"), *SessionId);
	}
	BO_LOG_NET(Log, "ClientTravel -> %s", *Addr);
	
	// 임시 우회: MoviePlayer가 ClientTravel 라이프사이클에서 SetIgnoreInput(false) 복구를 못 해
	// 보스맵 진입 후 viewport input lock이 고착됨. 시연 후 PostLoadMapWithWorld delegate 바인딩으로 본 fix.
	// FLoadingScreenAttributes LoadingScreenAttributes;
	// LoadingScreenAttributes.MinimumLoadingScreenDisplayTime=2.0f;
	// LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;
	// LoadingScreenAttributes.bMoviesAreSkippable = false;
	// LoadingScreenAttributes.WidgetLoadingScreen = SNew(SBlackoutLoadingScreen);
	// GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
	
	DisconnectLobby();
	PC->ClientTravel(Addr, TRAVEL_Absolute);
}

// 로비 WebSocket 연결. 이벤트 콜백 바인딩 + Connect 호출.
void UBlackoutMatchmakingSubsystem::ConnectLobby()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		BO_LOG_NET(Log, "이미 연결됨");
		return;
	}

	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(Settings->LobbyWsUrl);
	WebSocket->OnConnected().AddUObject(this, &UBlackoutMatchmakingSubsystem::HandleWsConnected);
	WebSocket->OnConnectionError().AddLambda([](const FString& Error)
	{
		BO_LOG_NET(Error, "WS 연결 에러: %s", *Error);
	});
	WebSocket->OnClosed().AddLambda([](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		BO_LOG_NET(Warning, "WS 종료 %d (%s, clean=%d)", StatusCode, *Reason, bWasClean);
	});
	WebSocket->OnMessage().AddUObject(this, &UBlackoutMatchmakingSubsystem::HandleWsMessage);
	WebSocket->Connect();
}

// WebSocket 닫기 + 현재 세션 식별자 초기화.
void UBlackoutMatchmakingSubsystem::DisconnectLobby()
{
	if (WebSocket.IsValid())
	{
		if (WebSocket->IsConnected())
		{
			WebSocket->Close();
		}
		WebSocket.Reset();
	}
	CurrentSessionId.Empty();
}

void UBlackoutMatchmakingSubsystem::Logout()
{
	
	if (AccessToken.IsEmpty() && CachedPlayerName.IsEmpty())
	{
		return;
	}
	BO_LOG_NET(Log,"로그아웃 - %s",*CachedPlayerName);
	AccessToken.Empty();
	CachedPlayerName.Empty();
	DisconnectLobby();
}


bool UBlackoutMatchmakingSubsystem::IsLobbyConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

void UBlackoutMatchmakingSubsystem::SetAuthHeader(const FHttpRequestRef& Request) const
{
	if (!AccessToken.IsEmpty())
	{
		Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AccessToken));
	}
}


// 로그인 응답 파싱. access_token 추출 후 ConnectLobby 자동 호출.
void UBlackoutMatchmakingSubsystem::OnLoginResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		BO_LOG_NET(Error, "Login 실패 - 응답 없음 (서버 다운/CORS/타임아웃)");
		CachedPlayerName.Empty();
		OnLoginComplete.Broadcast(false);
		return;
	}

	const int32 Status = Response->GetResponseCode();
	const FString BodyRaw = Response->GetContentAsString();
	BO_LOG_NET(Log, "Login 응답 %d: %s", Status, *BodyRaw);

	if (Status != 200 && Status != 201)
	{
		CachedPlayerName.Empty();
		OnLoginComplete.Broadcast(false);
		return;
	}

	TSharedPtr<FJsonObject> JsonObj;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyRaw);
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		BO_LOG_NET(Error, "응답 JSON 파싱 실패");
		CachedPlayerName.Empty();
		OnLoginComplete.Broadcast(false);
		return;
	}

	FString Token;
	if (!JsonObj->TryGetStringField(TEXT("access_token"), Token))
	{
		BO_LOG_NET(Error, "access_token 필드 없음");
		CachedPlayerName.Empty();
		OnLoginComplete.Broadcast(false);
		return;
	}

	AccessToken = Token;
	BO_LOG_NET(Log, "로그인 성공 - %s (토큰 길이 %d)", *CachedPlayerName, AccessToken.Len());
	ConnectLobby();
	OnLoginComplete.Broadcast(true);
}

// 매칭 시작 응답 파싱. session 필드에서 FBlackoutSessionInfo 채우고 join_session 예약.
void UBlackoutMatchmakingSubsystem::OnStartMatchmakingResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		OnMatchmakingError.Broadcast(0, TEXT("응답없음"));
		return;
	}

	const int32 Status = Response->GetResponseCode();
	const FString BodyRaw = Response->GetContentAsString();
	BO_LOG_NET(Log, "Start 응답 %d: %s", Status, *BodyRaw);

	if (Status != 200 && Status != 201)
	{
		OnMatchmakingError.Broadcast(Status, BodyRaw);
		return;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyRaw);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OnMatchmakingError.Broadcast(Status, TEXT("JSON 파싱 실패"));
		return;
	}

	const TSharedPtr<FJsonObject>* SessionObj = nullptr;
	if (!Root->TryGetObjectField(TEXT("session"), SessionObj) || !SessionObj)
	{
		OnMatchmakingError.Broadcast(Status, TEXT("session 필드 없음"));
		return;
	}

	FBlackoutSessionInfo Info;
	(*SessionObj)->TryGetStringField(TEXT("sessionId"), Info.SessionId);
	(*SessionObj)->TryGetStringField(TEXT("status"), Info.Status);
	(*SessionObj)->TryGetStringField(TEXT("serverId"), Info.ServerId);
	(*SessionObj)->TryGetStringField(TEXT("serverIp"), Info.ServerIp);
	(*SessionObj)->TryGetNumberField(TEXT("maxPlayers"), Info.MaxPlayers);
	(*SessionObj)->TryGetNumberField(TEXT("serverPort"), Info.ServerPort);

	const TArray<TSharedPtr<FJsonValue>>* PlayersArr = nullptr;
	if ((*SessionObj)->TryGetArrayField(TEXT("players"), PlayersArr))
	{
		for (const TSharedPtr<FJsonValue>& Player : *PlayersArr)
		{
			FString Name;
			if (Player.IsValid() && Player->TryGetString(Name))
			{
				Info.Players.Add(Name);
			}
		}
	}
	Root->TryGetBoolField(TEXT("isNewRoom"), Info.bIsNewRoom);

	BO_LOG_NET(Log, "매칭 시작 - sid=%s status=%s %d명 isNewRoom=%s",
		*Info.SessionId, *Info.Status, Info.Players.Num(), Info.bIsNewRoom ? TEXT("true") : TEXT("false"));

	SendJoinSessionMessage(Info.SessionId);
	OnMatchmakingStarted.Broadcast(Info);

	// 4번째 클라 안전망 — autoMatch 응답이 이미 playing 상태면 game_start WS 이벤트 손실 가능
	// (서버 측에서 정원 충족 직후 game_start emit, 이 클라는 아직 WS 룸 미참가 상태)
	if (Info.Status == TEXT("playing") && !Info.ServerIp.IsEmpty() && Info.ServerPort > 0)
	{
		BO_LOG_NET(Warning, "정원 충족 시점 진입 - game_start 이벤트 손실 가능, 직접 ClientTravel 트리거");
		if (bAutoTravelOnGameStart)
		{
			TravelToGameServer(Info.ServerIp, Info.ServerPort, Info.SessionId);
		}
		else
		{
			OnGameStart.Broadcast(Info.SessionId, Info.ServerIp, Info.ServerPort);
		}
	}
}

// 매칭 취소 응답. sessionDestroyed 값을 OnMatchmakingCancelled 로 전달.
void UBlackoutMatchmakingSubsystem::OnCancelMatchmakingResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		OnMatchmakingError.Broadcast(0, TEXT("응답없음"));
		return;
	}

	const int32 Status = Response->GetResponseCode();
	const FString BodyRaw = Response->GetContentAsString();
	BO_LOG_NET(Log, "Cancel 응답 %d: %s", Status, *BodyRaw);

	if (Status != 200)
	{
		OnMatchmakingError.Broadcast(Status, BodyRaw);
		return;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyRaw);
	bool bDestroyed = false;
	if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
	{
		Root->TryGetBoolField(TEXT("sessionDestroyed"), bDestroyed);
	}
	BO_LOG_NET(Log, "매칭 취소 - sessionDestroyed=%s", bDestroyed ? TEXT("true") : TEXT("false"));
	OnMatchmakingCancelled.Broadcast(bDestroyed);
}

// join_session 요청 처리. WebSocket 미연결이면 PendingSessionId 에 예약.
void UBlackoutMatchmakingSubsystem::SendJoinSessionMessage(const FString& SessionId)
{
	PendingSessionId = SessionId;
	if (IsLobbyConnected())
	{
		FlushPendingJoin();
	}
	else
	{
		BO_LOG_NET(Log, "WS 아직 미연결 — 연결 후 자동 전송 예약: %s", *SessionId);
	}
}

// 로비 WebSocket 메시지 분기. event 필드로 각 이벤트 델리게이트 Broadcast.
void UBlackoutMatchmakingSubsystem::HandleWsMessage(const FString& MessageStr)
{
	BO_LOG_NET(Log, "수신: %s", *MessageStr);

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MessageStr);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		return;
	}

	FString Event;
	if (!Root->TryGetStringField(TEXT("event"), Event))
	{
		return;
	}

	// 서버 ACK / keepalive 성격 이벤트는 클라에서 반응 없음.
	if (Event == TEXT("hi") || Event == TEXT("joined_session") || Event == TEXT("left_session") || Event == TEXT("pong"))
	{
		return;
	}

	const TSharedPtr<FJsonObject>* DataObj = nullptr;
	if (!Root->TryGetObjectField(TEXT("data"), DataObj) || !DataObj || !(*DataObj).IsValid())
	{
		return;
	}

	auto ExtractPlayers = [](const TSharedPtr<FJsonObject>& Obj) -> TArray<FString>
	{
		TArray<FString> Out;
		const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
		if (Obj->TryGetArrayField(TEXT("players"), Arr))
		{
			for (const TSharedPtr<FJsonValue>& Player : *Arr)
			{
				FString Name;
				if (Player.IsValid() && Player->TryGetString(Name))
				{
					Out.Add(Name);
				}
			}
		}
		return Out;
	};

	if (Event == TEXT("player_joined"))
	{
		FString Sid; int32 Count = 0;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		(*DataObj)->TryGetNumberField(TEXT("count"), Count);
		OnPlayerJoined.Broadcast(Sid, ExtractPlayers(*DataObj), Count);
		return;
	}

	if (Event == TEXT("player_left"))
	{
		FString Sid, PlayerName; int32 Count = 0;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		(*DataObj)->TryGetStringField(TEXT("playerName"), PlayerName);
		(*DataObj)->TryGetNumberField(TEXT("count"), Count);
		OnPlayerLeft.Broadcast(Sid, PlayerName, ExtractPlayers(*DataObj), Count);
		return;
	}

	if (Event == TEXT("game_start"))
	{
		FString Sid, Ip; int32 Port = 0;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		(*DataObj)->TryGetStringField(TEXT("serverIp"), Ip);
		(*DataObj)->TryGetNumberField(TEXT("serverPort"), Port);
		BO_LOG_NET(Log, "Game_Start - %s:%d", *Ip, Port);
		OnGameStart.Broadcast(Sid, Ip, Port);

		if (bAutoTravelOnGameStart)
		{
			TravelToGameServer(Ip, Port,Sid);
		}
		return;
	}

	if (Event == TEXT("session_cancelled"))
	{
		FString Sid;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		OnSessionCancelled.Broadcast(Sid);
		return;
	}

	if (Event == TEXT("matchmaking_failed"))
	{
		FString Sid, Reason;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		(*DataObj)->TryGetStringField(TEXT("reason"), Reason);
		OnMatchmakingFailed.Broadcast(Sid, Reason);
		return;
	}
}

// WebSocket 연결 성공 콜백. 예약된 join_session 송신.
void UBlackoutMatchmakingSubsystem::HandleWsConnected()
{
	BO_LOG_NET(Log, "WS 연결 성공");
	FlushPendingJoin();
}

void UBlackoutMatchmakingSubsystem::FlushPendingJoin()
{
	if (PendingSessionId.IsEmpty() || !IsLobbyConnected())
	{
		return;
	}

	const FString Payload = FString::Printf(
		TEXT("{\"event\":\"join_session\",\"data\":{\"sessionId\":\"%s\"}}"),
		*PendingSessionId);
	WebSocket->Send(Payload);
	CurrentSessionId = PendingSessionId;
	BO_LOG_NET(Log, "join_session 송신: %s", *CurrentSessionId);
	PendingSessionId.Empty();
}
