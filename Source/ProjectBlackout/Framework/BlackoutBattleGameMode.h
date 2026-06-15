#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "Core/BlackoutTypes.h"
#include "UObject/SoftObjectPath.h"
#include "GameplayTagContainer.h"
#include "BlackoutBattleGameMode.generated.h"

enum class EBlackoutMatchEndReason : uint8;
class ABlackoutPlayerCharacter;
class ABlackoutPlayerController;
class ABlackoutBossCharacter;
class ABOBossIntroSequencer;
class ABlackoutPlayerState;
/**
 * 전투 레벨 전용 GameMode. 전투 진입 자원 초기화 / 체크포인트 등록 / 파티 전멸 복귀 처리.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutBattleGameMode : public ABlackoutGameMode
{
	GENERATED_BODY()

public:
	// 매치 종료 트리거, 보스 사망 / 전원 이탈/ 타임아웃 등 외부 이벤트에서 호출
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	virtual void EndMatch(EBlackoutMatchEndReason Reason);

	// 중간 보스 처치 시 보스 사망 로직에서 호출. 보스 구현 후 연결된다.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	void OnBossDefeated();
	
	// 보스가 BeginPlay에서 자기 등록 
	void RegisterBoss(ABlackoutBossCharacter* Boss);

	// [테스트 전용] 콘솔에서 중간 보스 처치 시뮬레이션. 보스/4인 없이 루프 검증용.
	UFUNCTION(Exec)
	void BO_SimBossDefeated();

	// [테스트 전용] 콘솔에서 파티 전멸 시뮬레이션. 전투/사망 없이 Fast-Retry 회귀 검증용.
	UFUNCTION(Exec)
	void BO_SimPartyWipe();
	
	// 생존자 0명 감지 시 외부에서 호출. 현재 체크포인트로 전원 복귀 트리거.
	virtual void HandlePartyWipe() override;

	// 플레이어가 다운 타이머 만료로 완전 사망했을 때 호출. 생존자 0명 여부를 평가한다.
	void NotifyPlayerFullyDead(ABlackoutPlayerCharacter* DeadPlayer);
	
	// 다운 알림 
	void NotifyPlayerDowned(ABlackoutPlayerCharacter* DownedPlayer);
	
	void NotifyPlayerLoaded();   
	
	// ClientTravel URL SessionId DedicatedSessionSubsystem에 위임
	// 데디만 사용
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
	/** 현재 PS-> SelectdClassTag 기준으로 Pawn 재스폰 + Possess. ShelterPrep 캐릭터 변경에서 사용 */
	virtual void RespawnPlayerWithSelectedClass(APlayerController* InController) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Blackout|Battle|Players")
	TArray<TSubclassOf<APawn>> PlayerClassPool;
	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	void RegisterCutsceneManager(ABOBossIntroSequencer* InManager);
protected:
	// 플레이어 접속 시 전투 진입 자원 초기화 정책 적용 (LobbyToBattle).
	virtual void OnPlayerJoined(APlayerController* NewPlayer) override;
	
	// 전원 퇴장 grace 만료 시 베이스 가 호출. EndMatch , Idle 복귀
	virtual void HandleEmptyServerReset() override;

	// GameState 생성 직후 초기 상태를 WaitingForPlayers 로 세팅.
	virtual void InitGameState() override;

	// 매치 시작 후 신규 접속 거부 (WaitingForPlayers 가 아니면 난입 차단).
	virtual void PreLogin(const FString& Options, const FString& Address,
		const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// 항복 가결 시 복귀할 로비 맵 경로
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Battle")
	FSoftObjectPath LobbyMapPath;

	// 보스 처치 후 결과창을 표시하기 전까지 대기하는 시간입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Battle|Result", meta = (ClampMin = "0.0"))
	float MatchResultDisplayDelay = 3.0f;

	// 결과창 표시 후 아무도 진행을 확정하지 않아도 자동 이동되는 시간입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Battle|Result", meta = (ClampMin = "0.0"))
	float MatchResultAutoTravelDelay = 10.0f;

	void TravelToLobby(FLinearColor FadeColor);
	
	// 메인보스 클리어후 복귀 타이틀 맵 
	UPROPERTY(EditDefaultsOnly, Category="Blackout|Battle")
	FSoftObjectPath TitleMapPath;
	
	// ServerTravel 중복
	UPROPERTY(BlueprintReadOnly , Category="Blackout|Battle")
	bool bTravelInitiated  = false;
	
	virtual void OnSeamlessArrival(APlayerController* PC) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	bool bIsEncounterStarted = false;
	
	// 끊긴 플레이어 UniqueId -> SelectedClassTag 재접속 클래스 복원용 매치 종료시 Clear
	TMap<FString , FGameplayTag> ReconnectStash;
	
	// 로그인 ID -> stash 키. 빈 ID는 빈 문자열 (호출부에서 스킵)
	static FString MakeReconnectKey(const FString& AccountId);

	
	
public:
	virtual void Logout(AController* Exiting) override;

	/**
	 * 사망한 SpectatorController의 현재 ViewTarget을 기준으로 살아있는 다음/이전 아군으로 전환합니다.
	 * @param Direction -1=이전 / +1=다음.
	 */
	void CycleSpectateTargetForSpectator(ABlackoutPlayerController* SpectatorController, int32 Direction);

	/** 항복 투표 개시 */
	void StartSurrenderVote(ABlackoutPlayerController* Proposer);

	/** 찬성/반대 투표 처리 */
	void CastSurrenderVote(ABlackoutPlayerController* Voter, bool bAgree);

private:
	void EvaluateSurrenderVote();
	void HandleSurrenderSuccess();
	void HandleSurrenderFailed(bool bIsTimeout);
	void ClearSurrenderVotes();
	void SetAllPlayersSurrenderInputContextActive(bool bActive);
	void TimeoutSurrenderVote();
	
	void StartBossCombat();

	void BeginBossDefeatResultFlow(EBossType DefeatedBossType);
	void ShowMatchResultAfterDelay();
	void AutoTravelAfterMatchResult();
	void ExecuteMatchResultTravel();
	void SnapshotMatchResultParticipants(TArray<ABlackoutPlayerState*>& OutParticipants) const;
	
	void OnLoadingTimeout();   
	
	FTimerHandle LoadingTimeoutHandle;
	bool bBossCombatStarted = false;   

	UPROPERTY(EditDefaultsOnly, Category="Blackout|GameMode")
	float LoadingTimeout = 15.0f;     
	
	void TravelToTitle();

	FTimerHandle MatchResultDisplayTimerHandle;
	FTimerHandle MatchResultAutoTravelTimerHandle;
	EBossType PendingResultBossType = EBossType::Mid;

	FTimerHandle SurrenderVoteTimerHandle;

	void EvaluatePartyWipe();
	ABlackoutPlayerCharacter* FindInitialSpectateTarget(ABlackoutPlayerController* SpectatorController);
	void AssignSpectateTargetForDeadPlayer(ABlackoutPlayerController* SpectatorController);
	void RefreshSpectatorsForDeadTarget(ABlackoutPlayerCharacter* DeadTarget);

	int32 NextPlayerClassIndex =0;

	// 엔진이 ChoosePlayerStart / SpawnDefaultPawnFor / FindPlayerStart 경로에서
	// GetDefaultPawnClassForController_Implementation을 한 로그인당 여러 번 호출함.
	// 컨트롤러별 캐시로 동일 컨트롤러에 같은 클래스 반환 → 라운드로빈 카운터 한 번만 증가.
	UPROPERTY()
	TMap<TObjectPtr<AController>, TSubclassOf<APawn>> ControllerToClass;
	
	void DoTravelToLobby();
	
	void DoTravelToTitle();
	
	// 매치 종료 후 복귀 공통 정리 : 텔레메트리 run 종료 + 매치 진행 스테이지 리셋
	void ResetMatchProgressForReturn();

	// 데디 서버측 Idle 복귀 : 매치 인덱스 리셋 , 로비맵 ServerTravel  (타이틀/전원퇴장 공용)
	void ReturnServerToIdleLobby();
	
	FTimerHandle FadeTravelTimerHandle;
	
	class UBlackoutTelemetrySampler* GetTelemetrySampler() const;
	
	UPROPERTY(Transient)
	ABOBossIntroSequencer* CurrentCutsceneManager;
};
