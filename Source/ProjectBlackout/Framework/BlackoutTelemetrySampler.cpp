// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutTelemetrySampler.h"

#include "Core/BlackoutLog.h"
#include "Core/BlackoutLogCategories.h"
#include "BlackoutGameState.h"
#include "BlackoutNetworkSettings.h"
#include "BlackoutPlayerState.h"
#include "HttpModule.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "Interfaces/IHttpResponse.h"
#include "HAL/PlatformMisc.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"

// 샘플 1Hz / flush 30s / 버퍼상한 넘으면 로컬파일로
static constexpr float SAMPLE_INTERVAL_SECONDS = 1.0f;
static constexpr float FLUSH_INTERVAL_SECONDS = 30.0f;
static constexpr int32 TELEMETRY_BUFFER_CAP = 20000;
static constexpr int32 MAX_ITEMS_PER_FLUSH = 2000;

static FString BuildBatchJson(const TArray<FBlackoutMovementSample>& Samples , const TArray<FBlackoutMatchEvent>& Events)
{
	const TSharedRef <FJsonObject> Root = MakeShared<FJsonObject>();
	
	if (Samples.Num()>0)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		Values.Reserve(Samples.Num());
		for (const FBlackoutMovementSample& Sample : Samples)
		{
			Values.Add(MakeShared<FJsonValueObject>(FJsonObjectConverter::UStructToJsonObject(Sample)));
		}
		Root->SetArrayField(TEXT("samples"),Values);
	}
	if (Events.Num()>0)
	{
		TArray<TSharedPtr<FJsonValue>> Values;
		Values.Reserve(Events.Num());
		for (const FBlackoutMatchEvent& Event : Events)
		{
			Values.Add(MakeShared<FJsonValueObject>(FJsonObjectConverter::UStructToJsonObject(Event)));
		}
		Root->SetArrayField(TEXT("events"),Values);
	}
	FString Out;
	const TSharedRef<TJsonWriter<TCHAR , TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR> >::Create(&Out);
	FJsonSerializer::Serialize(Root, Writer);
	return Out;
}

void UBlackoutTelemetrySampler::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UBlackoutTelemetrySampler::Deinitialize()
{
	MovementSampleBuffer.Append(MoveTemp(InFlightMovementSamples));
	MatchEventBuffer.Append(MoveTemp(InFlightMatchEvents));
	InFlightMovementSamples.Empty();
	InFlightMatchEvents.Empty();
	// 비동기 HTTP 는 소멸후 못넘어감 , 남은 버퍼 동기로 로컬파일로
	if (MovementSampleBuffer.Num() > 0 || MatchEventBuffer.Num() > 0)
	{
		DumpToLocalFile();
	}
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SampleTimerHandle);
		World->GetTimerManager().ClearTimer(FlushTimerHandle);
	}

	Super::Deinitialize();
}

void UBlackoutTelemetrySampler::BeginRun()
{
	if (!IsDedicated())
	{
		return;
	}

	// RunId 없을때만 생성
	if (!RunId.IsValid())
	{
		RunId = FGuid::NewGuid();
		MatchStartTime = FDateTime::UtcNow();
		BO_LOG_NET(Log, "TelemetrySampler BeginRun — RunId=%s",
		           *RunId.ToString());
	}

	// 타이머는 tavel 마다 사라져서 매 BegunRun에서 현재 World에 다시 세팅
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.SetTimer(SampleTimerHandle, this,
		                      &UBlackoutTelemetrySampler::SampleTick,
		                      SAMPLE_INTERVAL_SECONDS, true);
		TimerManager.SetTimer(FlushTimerHandle, this,
		                      &UBlackoutTelemetrySampler::FlushPending,
		                      FLUSH_INTERVAL_SECONDS, true);
	}
}

void UBlackoutTelemetrySampler::EndRun()
{
	if (!RunId.IsValid())
	{
		return;
	}

	Flush(true);

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SampleTimerHandle);
		World->GetTimerManager().ClearTimer(FlushTimerHandle);
	}

	BO_LOG_NET(Log, "TelemetrySampler EndRun - RunId=%s 종료", *RunId.ToString());
	RunId.Invalidate();
}

void UBlackoutTelemetrySampler::FlushPending()
{
	if (!IsDedicated() || !RunId.IsValid())
	{
		return;
	}
	Flush(false);
}

void UBlackoutTelemetrySampler::AddEvent(ABlackoutPlayerCharacter* Player,
                                         const FString& EventType)
{
	if (!RunId.IsValid() || !Player)
	{
		return;
	}
	const ABlackoutPlayerState* PlayerState = Player->GetPlayerState<
		ABlackoutPlayerState>();

	FBlackoutMatchEvent Event;
	Event.EventId = FGuid::NewGuid().ToString();
	Event.MatchId = RunId.ToString();
	Event.AccountId = (PlayerState && !PlayerState->AccountId.IsEmpty())
		                  ? PlayerState->AccountId
		                  : (PlayerState
			                     ? FString::Printf(
				                     TEXT("pid:%d"), PlayerState->GetPlayerId())
			                     : FString());
	Event.TSec = (FDateTime::UtcNow() - MatchStartTime).GetTotalSeconds();
	const FVector Loc = Player->GetActorLocation();
	Event.X = Loc.X;
	Event.Y = Loc.Y;
	Event.Z = Loc.Z;
	Event.LevelName = GetNormalizedLevelName();
	Event.EventType = EventType;
	MatchEventBuffer.Add(MoveTemp(Event));
}

void UBlackoutTelemetrySampler::SampleTick()
{
	if (!RunId.IsValid())
	{
		return;
	}

	const UWorld* World = GetWorld();
	const AGameStateBase* GameStateBase = World
		                                      ? World->GetGameState()
		                                      : nullptr;
	if (!GameStateBase)
	{
		return;
	}

	const int32 TSec = GetRelativeSeconds();
	const FString LevelName = GetNormalizedLevelName();
	const FString MatchId = RunId.ToString();

	for (APlayerState* PlayerStateBase : GameStateBase->PlayerArray)
	{
		const ABlackoutPlayerState* PlayerState = Cast<ABlackoutPlayerState>(
			PlayerStateBase);
		const ABlackoutPlayerCharacter* Character = PlayerState
			                                            ? Cast<
				                                            ABlackoutPlayerCharacter>(
				                                            PlayerState->
				                                            GetPawn())
			                                            : nullptr;

		if (!Character)
		{
			continue; // 폰 없음 (관전/리스폰 대기) 
		}
		FBlackoutMovementSample Sample;
		Sample.MatchId = MatchId;
		Sample.AccountId = PlayerState->AccountId.IsEmpty()
			                   ? FString::Printf(
				                   TEXT("pid:%d"), PlayerState->GetPlayerId())
			                   : PlayerState->AccountId;
		Sample.TSec = TSec;
		const FVector Loc = Character->GetActorLocation();
		Sample.X = Loc.X;
		Sample.Y = Loc.Y;
		Sample.Z = Loc.Z;
		Sample.LevelName = LevelName;
		Sample.State = Character->IsDead()
			               ? TEXT("dead")
			               : Character->IsDowned()
			               ? TEXT("downed")
			               : TEXT("alive");
		MovementSampleBuffer.Add(MoveTemp(Sample));
	}

	if (MovementSampleBuffer.Num() > TELEMETRY_BUFFER_CAP)
	{
		DumpToLocalFile();
	}
}

void UBlackoutTelemetrySampler::Flush(bool bFinal)
{
	
	if (bFlushInFlight)
	{
		if (bFinal)
		{
			DumpToLocalFile();
		}
		return;
	}
	
	const int32 SampleCount = FMath::Min(MovementSampleBuffer.Num(), MAX_ITEMS_PER_FLUSH);
	const int32 EventCount = FMath::Min(MatchEventBuffer.Num(), MAX_ITEMS_PER_FLUSH);
	if (SampleCount == 0 && EventCount == 0)
	{
		return;
	}
	
	const FString ApiKey = FPlatformMisc::GetEnvironmentVariable(TEXT("BLACKOUT_API_KEY"));
	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	if (ApiKey.IsEmpty() || !Settings || Settings->ApiBaseUrl.IsEmpty())
	{
		BO_LOG_NET(Error , "Telemetry Flush - API 키/BaseURL 미설정, 로컬파일 대체")
		DumpToLocalFile();
		return;
	}
	
	InFlightMovementSamples.Append(MovementSampleBuffer.GetData() , SampleCount);
	MovementSampleBuffer.RemoveAt(0,SampleCount);
	InFlightMatchEvents.Append(MatchEventBuffer.GetData() , EventCount);
	MatchEventBuffer.RemoveAt(0,EventCount);
	
	const FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(FString::Printf(TEXT("%s/telemetry/batch"),*Settings->ApiBaseUrl));
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-server-api-key"),ApiKey);
	Request->SetContentAsString(BuildBatchJson(InFlightMovementSamples, InFlightMatchEvents));
	Request->OnProcessRequestComplete().BindUObject(this , &UBlackoutTelemetrySampler::OnFlushResponse , bFinal);
	Request->ProcessRequest();
	bFlushInFlight = true;
	
	BO_LOG_NET(Log, "Telemetry Flush — samples=%d events=%d final=%d",
			   SampleCount, EventCount, bFinal ? 1 : 0);
}

void UBlackoutTelemetrySampler::OnFlushResponse(FHttpRequestPtr Request,
                                                FHttpResponsePtr Response,
                                                bool bSucceeded ,
                                                bool bFinal)
{
	bFlushInFlight = false;
	
	const int32 Status = (bSucceeded && Response.IsValid()) ? Response->GetResponseCode() :0;
	
	if (Status >= 200 && Status < 300)
	{
		InFlightMovementSamples.Empty();
		InFlightMatchEvents.Empty();
		
		if (bFinal && (MovementSampleBuffer.Num() >0 || MatchEventBuffer.Num() > 0))
		{
			Flush(true);
		}
		return;
	}
	
	// 실패
	BO_LOG_NET(Warning, "Telemetry Flush 실패 — status=%d final=%d, 버퍼 복귀",
				   Status, bFinal ? 1 : 0);
	MovementSampleBuffer.Append(MoveTemp(InFlightMovementSamples));
	MatchEventBuffer.Append(MoveTemp(InFlightMatchEvents));
	InFlightMovementSamples.Empty();
	InFlightMatchEvents.Empty();
	
	const bool bPermanentFailure = Status >= 400 && Status < 500;
	if (bFinal || bPermanentFailure || !RunId.IsValid())
	{
		DumpToLocalFile();
	}
}

void UBlackoutTelemetrySampler::DumpToLocalFile()
{
	const int32 SampleCount = MovementSampleBuffer.Num();
	const int32 EventCount = MatchEventBuffer.Num();
	if (SampleCount ==0 && EventCount==0)
	{
		return;
	}
	
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("Telemetry");
	IFileManager::Get().MakeDirectory(*Dir, true);
	static int DumpCounter;
	const FString FilePath = Dir / FString::Printf(TEXT("telemetry_%s_%s_%d.json"), * RunId.ToString(EGuidFormats::Digits) , *FDateTime::UtcNow().ToString(), DumpCounter++);
	const FString Json = BuildBatchJson(MovementSampleBuffer , MatchEventBuffer);
	if (FFileHelper::SaveStringToFile(Json, *FilePath))
	{
		BO_LOG_NET(Warning , "Telemetry 로컬파일 -samples=%d events=%d -> %s",SampleCount, EventCount, *FilePath);
	}
	else
	{
		// 파일도 실패
		BO_LOG_NET(Error, "Telemetry 유실 — 파일 저장 실패 samples=%d events=%d",
				   SampleCount, EventCount);
	}
	MovementSampleBuffer.Empty();
	MatchEventBuffer.Empty();
}

bool UBlackoutTelemetrySampler::IsDedicated() const
{
	const UWorld* World = GetWorld();
	return World && World->GetNetMode() == NM_DedicatedServer;
}

int32 UBlackoutTelemetrySampler::GetRelativeSeconds() const
{
	const FTimespan Elapsed = FDateTime::UtcNow() - MatchStartTime;
	return FMath::Max(0, FMath::FloorToInt(Elapsed.GetTotalSeconds()));
}

FString UBlackoutTelemetrySampler::GetNormalizedLevelName() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return FString();
	}

	return UWorld::RemovePIEPrefix(World->GetMapName());
}
