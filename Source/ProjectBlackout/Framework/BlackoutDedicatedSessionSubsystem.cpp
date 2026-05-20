// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutDedicatedSessionSubsystem.h"

#include "Core/BlackoutLog.h"
#include "Core/BlackoutLogCategories.h"

#include "BlackoutNetworkSettings.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMisc.h"
#include "TimerManager.h"
#include "GameFramework/GameStateBase.h"
#include "BlackoutGameState.h"
#include "Core/BlackoutTypes.h"

// Heartbeat 송신 주기 — 매칭서버 TTL(90s) 의 1/3. 3회 누락 허용.
static constexpr float HEARTBEAT_INTERVAL_SECONDS = 30.0f;

void UBlackoutDedicatedSessionSubsystem::Initialize(
	FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UWorld* World = GetWorld();
	const ENetMode NetMode = World? World->GetNetMode(): NM_Standalone;
	
	BO_LOG_NET(Log, "DedicatedSessionSubsystem Initialize - NetMode=%d",
	           static_cast<int32>(NetMode));
	
	if (NetMode==NM_DedicatedServer)
	{
		RegisterToMatchmakingServer();
	}
}

void UBlackoutDedicatedSessionSubsystem::Deinitialize()
{
	StopHeartbeatLoop();
	CurrentSessionId.Empty();
	Super::Deinitialize();
}

void UBlackoutDedicatedSessionSubsystem::SetSessionId(
	const FString& InSessionId)
{
	CurrentSessionId = InSessionId;
	BO_LOG_NET(Log,"DedicatedSession SessionId 보관 - %s",*CurrentSessionId);
}

void UBlackoutDedicatedSessionSubsystem::ReportFinishToMatchmakingServer()
{
	// 데디 전용 가드 - PIE / Standalone / Listen Server 에서 EndMatch 호출 시 차단
	const UWorld* World = GetWorld();
	
	if (World && World->GetNetMode()!=NM_DedicatedServer)
	{
		BO_LOG_NET(Warning,"ReportFinish - 데디 환경 아님 (NetMode=%d), skip",static_cast<int32>(World->GetNetMode()));
		return;
	}
	
	if (CurrentSessionId.IsEmpty())
	{
		BO_LOG_NET(Warning,"ReportFinish -SessionId 없음, skip");
		return;
	}
	
	// API 키 - 환경변수만
	const FString ApiKey =FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKOUT_API_KEY"));
	if (ApiKey.IsEmpty())
	{
		BO_LOG_NET(Error, "ReportFinish - BLACKOUT_API_KEY 환경변수 없음, skip");
		return;
	}
	
	// 매칭서버 URL
	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings || Settings->ApiBaseUrl.IsEmpty())
	{
		BO_LOG_NET(Error, "ReportFinish - ApiBaseUrl 미설정");
		return;
	}
	
	const FString Url = FString::Printf(TEXT("%s/sessions/%s/finish"),*Settings->ApiBaseUrl,*CurrentSessionId);
	
	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-server-api-key"), ApiKey);
	Request->OnProcessRequestComplete().BindUObject(this , &UBlackoutDedicatedSessionSubsystem::OnFinishResponse);
	
	Request->ProcessRequest();
	BO_LOG_NET(Log, "ReportFinish 요청 - %s",*Url);
}

void UBlackoutDedicatedSessionSubsystem::OnFinishResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		BO_LOG_NET(Error, "ReportFinish 응답 실패 (네트워크 / 타임아웃)");
		return;
	}
	
	const int32 Status = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();
	
	if (Status >=200 && Status <300)
	{
		BO_LOG_NET(Log,"ReportFinishg 성공 - %d %s",Status, *Body);
		CurrentSessionId.Empty(); // 다음 매치
	}
	else
	{
		BO_LOG_NET(Error,"ReportFinish 실패 - %d %s",Status, *Body);
	}
	
}

void UBlackoutDedicatedSessionSubsystem::RegisterToMatchmakingServer()
{
	const FString Publicip = FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKOUT_PUBLIC_IP"));
	const FString PortStr = FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKOUT_PUBLIC_PORT"));
	
	if (Publicip.IsEmpty() || PortStr.IsEmpty())
	{
		BO_LOG_NET(Error, "Register - BLACKOUT_PUBLIC_IP / BLACKOUT_PUBLIC_PORT 환경변수 없음");
		return;
	}
	
	const int32 PublicPort = FCString::Atoi(*PortStr);
	if (PublicPort <=0)
	{
		BO_LOG_NET(Error, "Register - BLACKOUT_PUBLIC_PORT 파싱 실패: %s", *PortStr);
		return;
	}
	
	const FString Apikey = FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKOUT_API_KEY"));
	if (Apikey.IsEmpty())
	{
		BO_LOG_NET(Error, "Register - BLACKOUT_API_KEY 환경변수 없음");
		return;
	}
	
	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings || Settings->ApiBaseUrl.IsEmpty())
	{
		BO_LOG_NET(Error, "Register - ApiBaseUrl 미설정");
		return;
	}
	
	const FString Url =FString::Printf(TEXT("%s/servers/register"), *Settings->ApiBaseUrl);
	const FString Body = FString::Printf(TEXT("{\"ip\":\"%s\",\"port\":%d}"),*Publicip,PublicPort);

	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-server-api-key"), Apikey);
	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindUObject(this, &UBlackoutDedicatedSessionSubsystem::OnRegisterResponse);
	
	Request->ProcessRequest();
	BO_LOG_NET(Log, "Register 요청 - %s body=%s", *Url, *Body);
}

void UBlackoutDedicatedSessionSubsystem::OnRegisterResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	
	if (!bSucceeded || !Response.IsValid())
	{
		BO_LOG_NET(Error, "Register 응답 실패 (네트워크 / 타임아웃)");
		return;
	}
	
	const int32 Status = Response->GetResponseCode();
	const FString Body = Response->GetContentAsString();
	
	if (Status < 200 || Status >= 300)
	{
		BO_LOG_NET(Error, "Register 실패 - %d %s", Status, *Body);
		return;
	}
	
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
	if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
	{
		JsonObject->TryGetStringField(TEXT("serverId"),ServerId);
		BO_LOG_NET(Log, "Register 성공 - serverId=%s", *ServerId);

		// ServerId 받은 직후 heartbeat loop 시작. 매칭서버가 첫 alive TTL 만료 전 갱신.
		StartHeartbeatLoop();
	}else
	{
		BO_LOG_NET(Warning, "Register 응답 JSON 파싱 실패 - %s", *Body);
	}
}

void UBlackoutDedicatedSessionSubsystem::StartHeartbeatLoop()
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		BO_LOG_NET(Error, "StartHeartbeatLoop - World 없음");
		return;
	}

	World->GetTimerManager().SetTimer(
		HeartbeatTimerHandle,
		FTimerDelegate::CreateUObject(this, &UBlackoutDedicatedSessionSubsystem::SendHeartbeat),
		HEARTBEAT_INTERVAL_SECONDS,
		/*bLoop=*/true);

	BO_LOG_NET(Log, "Heartbeat 루프 시작 (%.1fs 주기)", HEARTBEAT_INTERVAL_SECONDS);
}

void UBlackoutDedicatedSessionSubsystem::StopHeartbeatLoop()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
	}
}

void UBlackoutDedicatedSessionSubsystem::SendHeartbeat()
{
	if (ServerId.IsEmpty())
	{
		BO_LOG_NET(Warning, "Heartbeat - ServerId 없음, skip");
		return;
	}

	const FString ApiKey = FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKOUT_API_KEY"));
	if (ApiKey.IsEmpty())
	{
		BO_LOG_NET(Error, "Heartbeat - BLACKOUT_API_KEY 환경변수 없음, skip");
		return;
	}

	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (!Settings || Settings->ApiBaseUrl.IsEmpty())
	{
		BO_LOG_NET(Error, "Heartbeat - ApiBaseUrl 미설정");
		return;
	}

	// Payload 수집 — 매칭서버가 받는 정보 (information only, 권위는 매칭서버 측 server.status).
	int32 PlayerCount = 0;
	FString StatusStr = TEXT("idle");
	FString MapName = TEXT("");
	int32 UptimeSeconds = 0;

	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			PlayerCount = GS->PlayerArray.Num();
		}
		if (const ABlackoutGameState* BGS = World->GetGameState<ABlackoutGameState>())
		{
			// InCombat 계열은 playing, 그 외는 idle 로 단순화 (매칭서버 status 와 호환).
			const EBlackoutMatchState MS = BGS->CurrentMatchState;
			const bool bPlaying =
				MS == EBlackoutMatchState::Starting ||
				MS == EBlackoutMatchState::InCombatReady ||
				MS == EBlackoutMatchState::InCombat ||
				MS == EBlackoutMatchState::ShelterPrep ||
				MS == EBlackoutMatchState::MidBossCombat ||
				MS == EBlackoutMatchState::ShelterMid ||
				MS == EBlackoutMatchState::MainBossCombat;
			StatusStr = bPlaying ? TEXT("playing") : TEXT("idle");
		}
		MapName = World->GetMapName();
		UptimeSeconds = FMath::FloorToInt(World->GetTimeSeconds());
	}

	const FString Body = FString::Printf(
		TEXT("{\"playerCount\":%d,\"status\":\"%s\",\"mapName\":\"%s\",\"uptimeSeconds\":%d}"),
		PlayerCount, *StatusStr, *MapName, UptimeSeconds);

	const FString Url = FString::Printf(TEXT("%s/servers/%s/heartbeat"), *Settings->ApiBaseUrl, *ServerId);

	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-server-api-key"), ApiKey);
	Request->SetContentAsString(Body);
	Request->OnProcessRequestComplete().BindUObject(this, &UBlackoutDedicatedSessionSubsystem::OnHeartbeatResponse);

	Request->ProcessRequest();
}

void UBlackoutDedicatedSessionSubsystem::OnHeartbeatResponse(
	FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded)
{
	if (!bSucceeded || !Response.IsValid())
	{
		BO_LOG_NET(Warning, "Heartbeat 응답 실패 (네트워크 / 타임아웃) — 다음 주기 재시도");
		return;
	}

	const int32 Status = Response->GetResponseCode();
	if (Status < 200 || Status >= 300)
	{
		BO_LOG_NET(Warning, "Heartbeat 비정상 응답 - %d", Status);
		// 매칭서버에 ServerId 가 사라진 경우(예: 매칭서버 재시작) 재등록 가능.
		// 시연 후 보강 — 지금은 로그만.
	}
}
