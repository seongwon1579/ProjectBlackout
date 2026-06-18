// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: 매칭 대기 위젯·게임 시작 시 로딩 텍스트 전환 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutMatchmakingWidget.generated.h"

class UButton;
class UTextBlock;
struct FBlackoutSessionInfo;

// 매칭 대기 종료 결과 bSuccess=false 면 사용자 취소 / 매칭실패 / 에러.
// 성공 (game_start) 는 자동 ClientTravel로 위젯이 destroy
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingExited , bool,bSuccess);

/**
 * 매칭 대기 위젯
 * 진입 시 Subsystem-> StartMatchmaking 호출
 * WS 이벤트 구독 -> 상태 텍스트 / 인원 카운트 갱신
 * game_start 수신시 Subsystem 의 Client Travel에 의해 위젯 정리
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutMatchmakingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category="Blackout|Matchmaking")
	FOnMatchmakingExited OnMatchmakingExited;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StatusText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PlayerCountText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CancelButton;
	
private:
	UFUNCTION()
	void HandleCancelClicked();
	
	UFUNCTION()
	void HandleMatchmakingStarted(const FBlackoutSessionInfo& SessionInfo);
	
	UFUNCTION()
	void HandlePlayerJoined(const FString& SessionId , const TArray<FString>& Players, int32 Count);
	
	UFUNCTION()
	void HandlePlayerLeft(const FString& SessionId , const FString& PlayerName , const TArray<FString>& Players, int32 Count);
	
	UFUNCTION()
	void HandleMatchmakingFailed(const FString& SessionId , const FString& Reason);
	
	UFUNCTION()
	void HandleSessionCancelled(const FString& SessionId);
	
	UFUNCTION()
	void HandleMatchmakingError(int32 HttpStatus , const FString& Message);
	
	UFUNCTION()
	void HandleMatchmakingCancelled(bool bSessionDestroyed);
	
	UFUNCTION()
	void HandleGameStart(const FString& SessionId , const FString& ServerIp , int32 ServerPort );
	
	void RefreshPlayerCount(int32 Count);
	
	void ExitWithResult(bool bSuccess);
	
	int32 CachedMaxPlayers = 0;
};
