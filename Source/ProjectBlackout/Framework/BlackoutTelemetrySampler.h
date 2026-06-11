// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpFwd.h"
#include "BlackoutTelemetrySampler.generated.h"

class ABlackoutPlayerCharacter;

// 1Hz 위치 샘플 - movement_sample DTO
USTRUCT()
struct FBlackoutMovementSample
{
	GENERATED_BODY()
	
	UPROPERTY() FString MatchId;
	UPROPERTY() FString AccountId;
	UPROPERTY() int32 TSec = 0;
	UPROPERTY() double X = 0.0;
	UPROPERTY() double Y = 0.0;
	UPROPERTY() double Z = 0.0;
	UPROPERTY() FString LevelName;
	UPROPERTY() FString State;
};

// 이벤트 - match_event DTO
USTRUCT()
struct FBlackoutMatchEvent
{
	GENERATED_BODY()
	
	UPROPERTY() FString EventId;
	UPROPERTY() FString MatchId;
	UPROPERTY() FString AccountId;
	UPROPERTY() double  TSec = 0;
	UPROPERTY() double  X = 0.0;
	UPROPERTY() double  Y = 0.0;
	UPROPERTY() double  Z = 0.0;
	UPROPERTY() FString LevelName;
	UPROPERTY() FString EventType;
};

/**
 * 플레이어 위치 정보
 * 위치 정보 , 사망 /다운 이벤트를 백엔드로 
 * MatchId는 GameInstance에서 
 */
UCLASS()
class PROJECTBLACKOUT_API
	UBlackoutTelemetrySampler : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	// 매치 시작 , MatchId 없으면 생성 
	void BeginRun();
	
	// 매치 종료 / Idle 복귀 flush + MatchId 정리 + 타이머 정지 
	void EndRun();
	
	// 스테이지 전환 (중간 보스 -> 로비) 
	void FlushPending();
	
	// 사망 , 다운 이벤트 기록 
	void AddEvent(ABlackoutPlayerCharacter* Player , const FString& EventType);
	
private:
	void SampleTick();
	void Flush(bool bFinal);
	void OnFlushResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded , bool bFinal);
	void DumpToLocalFile();
	
	bool IsDedicated() const;
	int32 GetRelativeSeconds() const;
	FString GetNormalizedLevelName() const;
	
	FGuid RunId; // match_id
	FDateTime MatchStartTime;
	
	// 수집 버퍼 
	TArray<FBlackoutMovementSample> MovementSampleBuffer;
	TArray<FBlackoutMatchEvent> MatchEventBuffer;
	TArray<FBlackoutMovementSample> InFlightMovementSamples;
	TArray<FBlackoutMatchEvent> InFlightMatchEvents;
	
	FTimerHandle SampleTimerHandle;
	FTimerHandle FlushTimerHandle;
	bool bFlushInFlight = false;
};
