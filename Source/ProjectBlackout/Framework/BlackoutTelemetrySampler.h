// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: 플레이어 텔레메트리 데디 샘플러 — 1Hz 위치 샘플·사망/다운 이벤트 수집·배치 flush 백엔드 전송
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "HttpFwd.h"
#include "AI/Enum/BOBossPhase.h"
#include "BlackoutTelemetrySampler.generated.h"

class ABlackoutPlayerCharacter;
class AActor;

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

USTRUCT()
struct FBlackoutEventContext
{
	GENERATED_BODY()
	UPROPERTY() int32 Phase = 0;
	
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
	UPROPERTY() FBlackoutEventContext  Context; // 발생 시점 보스 페이즈(EBOBossPhase, 0=None). Ravager 전용
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
	
	// 보스 페이즈 전환 통지
	void RecordBossPhaseChange(EBOBossPhase NewPhase , AActor* Boss);
	
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
	EBOBossPhase CurrentBossPhase = EBOBossPhase::None;
};
