// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "Interfaces/IHttpRequest.h"
#include "HttpFwd.h"
#include "BlackoutDedicatedSessionSubsystem.generated.h"

/**
 * 데디 서버 측 매칭 세션 메타데이터 + 매칭 서버 통신 책임
 *  - BattleGameMode InitGame 에서 ClientTravel URL SessionId 옵션받아 보관
 *  - EndMatch 시점에 POST /sessions/:id/finish (종료) 호출
 *  
 *  데디 / 클라 양쪽 생성 , 사용은 데디만 
 *  클라 인스턴스의 Subsystem은 SessionId 빈상태로
 */
UCLASS()
class PROJECTBLACKOUT_API
	UBlackoutDedicatedSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual  void Deinitialize() override;
	
	// BattleGameMode InitGame 에서 호출 SessionId 보관
	UFUNCTION(BlueprintCallable, Category="Blackout|DedicatedSession")
	void SetSessionId(const FString& InSessionId);
	
	UFUNCTION(BlueprintPure,Category="Blackout|DedicatedSession")
	const FString& GetSessionId() const {return CurrentSessionId;}
	
	UFUNCTION(BlueprintPure,Category="Blackout|DedicatedSession")
	bool HasActiveSession() const {return !CurrentSessionId.IsEmpty();}
	
	// EndMatch 시점에 호출 POST /sessions/:id/finish
	void ReportFinishToMatchmakingServer();

	// EndMatch / 빈 서버 복귀 시 POST /servers/:id/idle — serverId 기반이라 세션 만료와 무관하게 markIdle
	void ReportIdleToMatchmakingServer();

private:
	// HTTP 응답 콜백
	void OnFinishResponse(FHttpRequestPtr Request, FHttpResponsePtr Response , bool bSucceeded);
	void OnIdleResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	// 데디 시작시 매칭 서버에 등록
	void RegisterToMatchmakingServer();
	void OnRegisterResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	// Heartbeat — 30초 주기로 POST /servers/:id/heartbeat. 매칭서버가 데디 liveness 감지.
	// Register 응답으로 ServerId 받은 직후 타이머 시작.
	void StartHeartbeatLoop();
	void StopHeartbeatLoop();
	void SendHeartbeat();
	void OnHeartbeatResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSucceeded);

	FTimerHandle HeartbeatTimerHandle;

	FString CurrentSessionId;
	FString ServerId; // /server/register 응답
};
