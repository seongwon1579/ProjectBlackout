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
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"

#include "MoviePlayer.h"
#include "UI/SBlackoutLoadingScreen.h"

// GameInstance 부팅 시 호출. WebSockets 모듈 로드 + NetworkSettings 기본값 적용.
void UBlackoutMatchmakingSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	FModuleManager::Get().LoadModuleChecked(TEXT("WebSockets"));

	if (const UBlackoutNetworkSettings* Settings = GetDefault<
		UBlackoutNetworkSettings>())
	{
		bAutoTravelOnGameStart = Settings->bAutoTravelOnGameStart;
		BO_LOG_NET(Log, "Matchmaking Subsystem 초기화 - API=%s WS=%s",
		           *Settings->ApiBaseUrl, *Settings->LobbyWsUrl);
	}

	// 로딩 화면 라이프사이클 명시 바인딩 — ClientTravel 시 자동 표시 + 맵 로드 완료 시 명시 종료.
	// ClientTravel 비동기 흐름에서 bAutoCompleteWhenLoadingCompletes 만으론 dismiss 보장 안 됨 → 명시 StopMovie 필요.
	PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(
		this, &UBlackoutMatchmakingSubsystem::HandlePreLoadMap);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UBlackoutMatchmakingSubsystem::HandlePostLoadMap);

	if (GEngine)
	{
		NetworkFailureHandle = GEngine->OnNetworkFailure().AddUObject(
			this, &UBlackoutMatchmakingSubsystem::HandleNetworkFailure);
	}
}

// GameInstance 종료 시 WebSocket 정리 + 토큰 폐기.
void UBlackoutMatchmakingSubsystem::Deinitialize()
{
	if (PreLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
		PreLoadMapHandle.Reset();
	}
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	DisconnectLobby();
	AccessToken.Empty();
	CachedPlayerName.Empty();

	if (GEngine && NetworkFailureHandle.IsValid())
	{
		GEngine->OnNetworkFailure().Remove(NetworkFailureHandle);
	}

	Super::Deinitialize();
}

// 맵 로드 직전 — MoviePlayer 에 Slate 로딩 위젯 셋업. PreLoadMap → PlayMovie 흐름이 엔진 내부에서 자동 진행.
void UBlackoutMatchmakingSubsystem::HandlePreLoadMap(const FString& MapName)
{
	BO_LOG_NET(Log, "PreLoadMap: %s — 로딩 화면 셋업", *MapName);

	FLoadingScreenAttributes LoadingScreenAttributes;
	LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 2.0f;
	LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = false;
	LoadingScreenAttributes.bMoviesAreSkippable = false;
	LoadingScreenAttributes.WidgetLoadingScreen = SNew(SBlackoutLoadingScreen);
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
}

// 맵 로드 완료 — MoviePlayer 명시 종료. viewport input lock 해소까지 보장.
// bAutoComplete 비활성 + 명시 StopMovie 패턴이 ClientTravel 흐름에서 가장 안정적.
void UBlackoutMatchmakingSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	BO_LOG_NET(Log, "PostLoadMapWithWorld — 로딩 화면 종료");

	if (GetMoviePlayer()->IsMovieCurrentlyPlaying())
	{
		GetMoviePlayer()->StopMovie();
	}
	GetMoviePlayer()->WaitForMovieToFinish();

	// 데지 접속 성공시 연결 주소 기억 
	// 자동 재접속 대상 
	if (LoadedWorld)
	{
		if (const UNetDriver* ND = LoadedWorld->GetNetDriver())
		{
			if (ND->ServerConnection) // 클라(서버 연결됨)
			{
				const FString Addr = ND->ServerConnection->
				                         LowLevelGetRemoteAddress(true);
				FString Ip, PortStr;
				if (Addr.Split(TEXT(":"), &Ip, &PortStr,
				               ESearchCase::IgnoreCase, ESearchDir::FromEnd))
				{
					LastServerIp = Ip;
					LastServerPort = FCString::Atoi(*PortStr);
					BO_LOG_NET(Log, "데디 접속 주소 기억: %s:%d", *LastServerIp,
					           LastServerPort);
					
					if (ReconnectAttempts >0)
					{
						BO_LOG_NET(Log, "재접속 성공 — 자동 재접속 중단");
						ReconnectAttempts = 0;
						bIsReconnecting = false;
						if (UGameInstance* GI = GetGameInstance())
						{
							GI->GetTimerManager().ClearTimer(ReconnectTimerHandle);
						}
					}
				}
			}
		}
	}
}

// POST /auth/login — JSON body로 계정 전송. 응답은 OnLoginResponse.
void UBlackoutMatchmakingSubsystem::Login(const FString& PlayerName,
                                          const FString& Password)
{
	const UBlackoutNetworkSettings* Settings = GetDefault<
		UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	const FString BodyStr = FString::Printf(
		TEXT("{\"playerName\":\"%s\",\"password\":\"%s\"}"),
		*PlayerName, *Password);

	const TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->ApiBaseUrl + TEXT("/auth/login"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(BodyStr);
	Request->OnProcessRequestComplete().BindUObject(
		this, &UBlackoutMatchmakingSubsystem::OnLoginResponse);

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

	const UBlackoutNetworkSettings* Settings = GetDefault<
		UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->ApiBaseUrl + TEXT("/matchmaking/start"));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	SetAuthHeader(Request);
	Request->OnProcessRequestComplete().BindUObject(
		this, &UBlackoutMatchmakingSubsystem::OnStartMatchmakingResponse);
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

	const UBlackoutNetworkSettings* Settings = GetDefault<
		UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Settings->ApiBaseUrl + TEXT("/matchmaking"));
	Request->SetVerb(TEXT("DELETE"));
	SetAuthHeader(Request);
	Request->OnProcessRequestComplete().BindUObject(
		this, &UBlackoutMatchmakingSubsystem::OnCancelMatchmakingResponse);
	Request->ProcessRequest();

	BO_LOG_NET(Log, "CancelMatchmaking 요청");
}

// 데디 IP:Port 로 ClientTravel. 호출 전 로비 WebSocket 정리.
void UBlackoutMatchmakingSubsystem::TravelToGameServer(
	const FString& ServerIp, int32 ServerPort, const FString& SessionId)
{
	LastServerIp = ServerIp;
	LastServerPort = ServerPort;
	LastSessionId = SessionId;

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

	// 메인메뉴/매칭 위젯이 SetInputModeUIOnly 로 viewport 를 잠가둔 상태로 Travel 진입 시
	// 새 World 의 PlayerController 가 입력을 못 받음. Travel 직전 명시 GameOnly 로 복구.
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}

	FString Addr = FString::Printf(TEXT("%s:%d"), *ServerIp, ServerPort);
	if (!SessionId.IsEmpty())
	{
		Addr += FString::Printf(TEXT("?SessionId=%s"), *SessionId);
		
	}
	if (!CachedPlayerName.IsEmpty())
	{
		Addr +=FString::Printf(TEXT("?Acc=%s"), *CachedPlayerName);
	}
	BO_LOG_NET(Log, "ClientTravel -> %s", *Addr);

	// 로딩 화면은 PreLoadMap delegate(Initialize 에서 바인딩) 가 자동 셋업.
	// 라이프사이클 종료는 PostLoadMapWithWorld delegate 가 명시 StopMovie 로 처리.
	DisconnectLobby();
	PC->ClientTravel(Addr, TRAVEL_Absolute);
}

void UBlackoutMatchmakingSubsystem::TravelToActiveSession()
{
	if (ActiveSessionIp.IsEmpty() || ActiveSessionPort <= 0)
	{
		BO_LOG_NET(Warning, "TravelToActiveSession — 저장된 세션 없음");
		return;
	}
	TravelToGameServer(ActiveSessionIp , ActiveSessionPort , ActiveSessionId);
}

// 로비 WebSocket 연결. 이벤트 콜백 바인딩 + Connect 호출.
void UBlackoutMatchmakingSubsystem::ConnectLobby()
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		BO_LOG_NET(Log, "이미 연결됨");
		return;
	}

	const UBlackoutNetworkSettings* Settings = GetDefault<
		UBlackoutNetworkSettings>();
	if (!Settings) { return; }

	WebSocket = FWebSocketsModule::Get().CreateWebSocket(Settings->LobbyWsUrl);
	WebSocket->OnConnected().AddUObject(
		this, &UBlackoutMatchmakingSubsystem::HandleWsConnected);
	WebSocket->OnConnectionError().AddLambda([](const FString& Error)
	{
		BO_LOG_NET(Error, "WS 연결 에러: %s", *Error);
	});
	WebSocket->OnClosed().AddLambda(
		[](int32 StatusCode, const FString& Reason, bool bWasClean)
		{
			BO_LOG_NET(Warning, "WS 종료 %d (%s, clean=%d)", StatusCode, *Reason,
			           bWasClean);
		});
	WebSocket->OnMessage().AddUObject(
		this, &UBlackoutMatchmakingSubsystem::HandleWsMessage);
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
	BO_LOG_NET(Log, "로그아웃 - %s", *CachedPlayerName);
	AccessToken.Empty();
	CachedPlayerName.Empty();
	DisconnectLobby();
}


bool UBlackoutMatchmakingSubsystem::IsLobbyConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

void UBlackoutMatchmakingSubsystem::CheckActiveSession()
{
	if (!IsLoggedIn())
	{
		BO_LOG_NET(Log, "CheckActiveSession — 로그인 안 됨, skip");
		return;
	}
	
	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings)
	{
		return;
	}
	
	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request ->SetURL(Settings->ApiBaseUrl + TEXT("/me/active-session"));
	Request->SetVerb(TEXT("GET"));
	SetAuthHeader(Request);
	Request->OnProcessRequestComplete().BindUObject(this, &UBlackoutMatchmakingSubsystem::OnCheckActiveSessionResponse);
	Request->ProcessRequest();
	
	BO_LOG_NET(Log, "CheckActiveSession 요청");

}

void UBlackoutMatchmakingSubsystem::ManualReconnect()
{
	ReconnectAttempts = 0;
	bIsReconnecting  =true;
	AttemptReconnect();
}

void UBlackoutMatchmakingSubsystem::SetAuthHeader(
	const FHttpRequestRef& Request) const
{
	if (!AccessToken.IsEmpty())
	{
		Request->SetHeader(
			TEXT("Authorization"),
			FString::Printf(TEXT("Bearer %s"), *AccessToken));
	}
}


// 로그인 응답 파싱. access_token 추출 후 ConnectLobby 자동 호출.
void UBlackoutMatchmakingSubsystem::OnLoginResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
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
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory
		<>::Create(BodyRaw);
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
	BO_LOG_NET(Log, "로그인 성공 - %s (토큰 길이 %d)", *CachedPlayerName,
	           AccessToken.Len());
	ConnectLobby();
	OnLoginComplete.Broadcast(true);
}

// 매칭 시작 응답 파싱. session 필드에서 FBlackoutSessionInfo 채우고 join_session 예약.
void UBlackoutMatchmakingSubsystem::OnStartMatchmakingResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
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
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory
		<>::Create(BodyRaw);
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
	           *Info.SessionId, *Info.Status, Info.Players.Num(),
	           Info.bIsNewRoom ? TEXT("true") : TEXT("false"));

	SendJoinSessionMessage(Info.SessionId);
	OnMatchmakingStarted.Broadcast(Info);

	// 4번째 클라 안전망 — autoMatch 응답이 이미 playing 상태면 game_start WS 이벤트 손실 가능
	// (서버 측에서 정원 충족 직후 game_start emit, 이 클라는 아직 WS 룸 미참가 상태)
	if (Info.Status == TEXT("playing") && !Info.ServerIp.IsEmpty() && Info.
		ServerPort > 0)
	{
		BO_LOG_NET(Warning,
		           "정원 충족 시점 진입 - game_start 이벤트 손실 가능, 직접 ClientTravel 트리거");
		if (bAutoTravelOnGameStart)
		{
			TravelToGameServer(Info.ServerIp, Info.ServerPort, Info.SessionId);
		}
		else
		{
			OnGameStart.Broadcast(Info.SessionId, Info.ServerIp,
			                      Info.ServerPort);
		}
	}
}

// 매칭 취소 응답. sessionDestroyed 값을 OnMatchmakingCancelled 로 전달.
void UBlackoutMatchmakingSubsystem::OnCancelMatchmakingResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
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
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory
		<>::Create(BodyRaw);
	bool bDestroyed = false;
	if (FJsonSerializer::Deserialize(Reader, Root) && Root.IsValid())
	{
		Root->TryGetBoolField(TEXT("sessionDestroyed"), bDestroyed);
	}
	BO_LOG_NET(Log, "매칭 취소 - sessionDestroyed=%s",
	           bDestroyed ? TEXT("true") : TEXT("false"));
	OnMatchmakingCancelled.Broadcast(bDestroyed);
}

void UBlackoutMatchmakingSubsystem::OnCheckActiveSessionResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		BO_LOG_NET(Warning, "CheckActiveSession 응답 실패");
		return;
	}
	const int32 Status = Response->GetResponseCode();
	if (Status < 200 || Status >= 300)
	{
		BO_LOG_NET(Warning, "CheckActiveSession 비정상 - %d", Status);
		return;
	}
	
	// 진행 세션 없으면 null / 빈 바디 -> 파싱 실패로 버튼 안나옴
	TSharedPtr<FJsonObject> Json;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		BO_LOG_NET(Log, "CheckActiveSession — 진행 세션 없음");
		return;
	}
	
	FString Ip, SessionId;
	int32 Port = 0;
	Json->TryGetStringField(TEXT("dediIp"),Ip);
	Json->TryGetNumberField(TEXT("port"),Port);
	Json->TryGetStringField(TEXT("sessionId"),SessionId);
	
	if (Ip.IsEmpty() || Port<= 0)
	{
		BO_LOG_NET(Log, "CheckActiveSession — 데디 주소 없음(미배정)");
		return;
	}
	ActiveSessionIp = Ip;
	ActiveSessionPort = Port;
	ActiveSessionId = SessionId;
	BO_LOG_NET(Log, "재접속 가능 세션: %s:%d (%s)", *Ip, Port, *SessionId);
	OnActiveSessionFound.Broadcast();
	
}

// join_session 요청 처리. WebSocket 미연결이면 PendingSessionId 에 예약.
void UBlackoutMatchmakingSubsystem::SendJoinSessionMessage(
	const FString& SessionId)
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
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(
		MessageStr);
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
	if (Event == TEXT("hi") || Event == TEXT("joined_session") || Event ==
		TEXT("left_session") || Event == TEXT("pong"))
	{
		return;
	}

	const TSharedPtr<FJsonObject>* DataObj = nullptr;
	if (!Root->TryGetObjectField(TEXT("data"), DataObj) || !DataObj || !(*
		DataObj).IsValid())
	{
		return;
	}

	auto ExtractPlayers = [](
		const TSharedPtr<FJsonObject>& Obj) -> TArray<FString>
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
		FString Sid;
		int32 Count = 0;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		(*DataObj)->TryGetNumberField(TEXT("count"), Count);
		OnPlayerJoined.Broadcast(Sid, ExtractPlayers(*DataObj), Count);
		return;
	}

	if (Event == TEXT("player_left"))
	{
		FString Sid, PlayerName;
		int32 Count = 0;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		(*DataObj)->TryGetStringField(TEXT("playerName"), PlayerName);
		(*DataObj)->TryGetNumberField(TEXT("count"), Count);
		OnPlayerLeft.Broadcast(Sid, PlayerName, ExtractPlayers(*DataObj),
		                       Count);
		return;
	}

	if (Event == TEXT("game_start"))
	{
		FString Sid, Ip;
		int32 Port = 0;
		(*DataObj)->TryGetStringField(TEXT("sessionId"), Sid);
		(*DataObj)->TryGetStringField(TEXT("serverIp"), Ip);
		(*DataObj)->TryGetNumberField(TEXT("serverPort"), Port);
		BO_LOG_NET(Log, "Game_Start - %s:%d", *Ip, Port);
		OnGameStart.Broadcast(Sid, Ip, Port);

		if (bAutoTravelOnGameStart)
		{
			TravelToGameServer(Ip, Port, Sid);
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

void UBlackoutMatchmakingSubsystem::HandleNetworkFailure(UWorld* World,
	UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	// 비정상 끊김만 자동 재접속 대상, 정상 종료/기타는 무시
	const bool bAbnormal = FailureType == ENetworkFailure::ConnectionLost ||
		FailureType == ENetworkFailure::ConnectionTimeout;

	if (!bAbnormal || LastServerIp.IsEmpty())
	{
		BO_LOG_NET(Log, "NetworkFailure 무시: Type=%d LastServer=%s",
		           (int32)FailureType, *LastServerIp);
		return;
	}
	if (bIsReconnecting)
	{
		BO_LOG_NET(Log, "재접속 사이클 진행 중 — 추가 NetworkFailure 무시(Type=%d)", (int32)FailureType);
		return;
	}

	BO_LOG_NET(Warning, "비정상 끊김 감지(Type=%d) — 자동 재접속 대상 %s:%d",
	           (int32)FailureType, *LastServerIp, LastServerPort);

	ReconnectAttempts = 0;
	bIsReconnecting = true;
	ScheduleReconnect();
}

void UBlackoutMatchmakingSubsystem::ScheduleReconnect()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		GI->GetTimerManager().SetTimer(ReconnectTimerHandle, this,
		                               &UBlackoutMatchmakingSubsystem::AttemptReconnect,
		                               ReconnectInterval, false);
	}
}

void UBlackoutMatchmakingSubsystem::AttemptReconnect()
{
	if (ReconnectAttempts >= MaxReconnectAttempts)
	{
		BO_LOG_NET(Warning, "자동 재접속 %d회 모두 실패 — 수동 재접속 대기",
				   MaxReconnectAttempts);
		OnReconnectFailed.Broadcast();
		return;
	}
	++ReconnectAttempts;
	BO_LOG_NET(Log, "자동 재접속 시도 %d/%d -> %s:%d",
	ReconnectAttempts, MaxReconnectAttempts, *LastServerIp, LastServerPort);
	TravelToGameServer(LastServerIp ,LastServerPort, LastSessionId);
	ScheduleReconnect(); // 다음 라운드 예약 (성공 시 PostLoadMap 에서 중단)
}
