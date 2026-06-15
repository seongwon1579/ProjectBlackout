#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlackoutGameMode.generated.h"

enum class EBlackoutMatchState : uint8;
class APlayerController;


UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABlackoutGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|GameMode")
	virtual void HandlePartyWipe();
	
	UFUNCTION(BlueprintCallable , Category="Blackout|GameMode")
	virtual void RespawnPlayerWithSelectedClass(APlayerController* InController);

	// 정원 충족 + 전원 bIsReady == true 조건 검사. Lobby / Battle 공용.
	UFUNCTION(BlueprintCallable, Category = "Blackout|GameMode")
	virtual bool AllPlayersReady() const;

	// PlayerController::Server_SetReady 처리 직후 호출. AllPlayersReady 성립 시 OnAllPlayersReady 훅 실행.
	UFUNCTION(BlueprintCallable, Category = "Blackout|GameMode")
	virtual void NotifyReadyChanged();

protected:
	// 매치 시작 시 URL 옵션(?SessionId=...)을 파싱해 MatchmakingSessionId에 보관한다.
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// 클라가 매칭으로 처음 접속할 때 URL ?SessionId=... 옵션을 잡아 DedicatedSessionSubsystem 에 저장.
	virtual void PreLogin(const FString& Options, const FString& Address,
		const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	
	// 접속시 URL ?Acc (로그인 ID) 파싱 -> PlayerState에 보관 재접속 stash 키 소스
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;

	virtual void Logout(AController* Exiting) override;
	
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	

	// 자식 GameMode(Lobby/Battle)가 공통 집계 뒤 확장 로직을 붙이는 훅.
	virtual void OnPlayerJoined(APlayerController* NewPlayer) {}
	virtual void OnPlayerLeft(AController* Exiting) {}
	
	// seamless 도착 시 공통 집계 뒤 자식이 붙는 훅
	virtual  void OnSeamlessArrival(APlayerController* PC) {};
	
	// 전원 퇴장 (정원 0) 이 grace동안 유지되면 호출
	virtual void HandleEmptyServerReset(){}
	
	// 정원 0 grace 만료 콜백. 여전히 0이면 HandleEmptyServerReset 실행
	void ConfirmEmptyServer();
	
	// 전원 Ready 성립 시 자식 GameMode 가 override 하여 액션을 정의하는 훅 (Lobby: StartBattle / Battle: 보스 활성).
	virtual void OnAllPlayersReady() {}

	// 매치 상태 전이 단일 권위. 허용된 전이만 GameState 에 반영하고 거부 시 경고 로그를 남긴다.
	virtual void TransitionTo(EBlackoutMatchState NewState);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|GameMode")
	int32 MaxPlayers = 4;

	// 매칭 API(Nest.js)가 데디 URL 옵션으로 넘긴 세션 식별자. 세션 CRUD와 1:1 매핑.
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|GameMode")
	FString MatchmakingSessionId;

	// 현재 접속 중인 플레이어 컨트롤러. 서버 전용이므로 리플리케이션 대상 아님.
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|GameMode")
	TArray<TObjectPtr<APlayerController>> ConnectedPlayers;
	
	// 정원 0 감지후 Idle 복귀까지 유예
	UPROPERTY(EditDefaultsOnly , Category="Blackout|GameMode")
	float EmptyServerGracePeriod =10.0f;
	
	FTimerHandle EmptyServerGraceHandle;
	
	// 레벨 전환 직전, 접속 중 전 클라에 화면 페이드아웃 브로드캐스트. bHoldUntilReady=true 면 도착 후 ready 게이트까지 fade-in 보류(로비->보스)
	void BroadcastScreenFadeOut(FLinearColor FadeColor, bool bHoldUntilReady = false);
	
	// 페이드아웃 후 실제 travel 까지 서버대기 
	UPROPERTY(EditDefaultsOnly, Category="Blackout|Transition")
	float FadeOutTravelDelay = 1.5f;
	

};
