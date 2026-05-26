#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "Interfaces/BlackoutArenaResettable.h"
#include "BlackoutBattleGameMode.generated.h"

enum class EBlackoutMatchEndReason : uint8;
class ABlackoutPlayerCharacter;
class ABlackoutPlayerController;
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
	void OnMidBossDefeated();

	// [테스트 전용] 콘솔에서 중간 보스 처치 시뮬레이션. 보스/4인 없이 루프 검증용.
	UFUNCTION(Exec)
	void BO_SimMidBossDefeated();

	// [테스트 전용] 콘솔에서 파티 전멸 시뮬레이션. 전투/사망 없이 Fast-Retry 회귀 검증용.
	UFUNCTION(Exec)
	void BO_SimPartyWipe();
	
	// 생존자 0명 감지 시 외부에서 호출. 현재 체크포인트로 전원 복귀 트리거.
	virtual void HandlePartyWipe() override;

	// 플레이어가 다운 타이머 만료로 완전 사망했을 때 호출. 생존자 0명 여부를 평가한다.
	void NotifyPlayerFullyDead(ABlackoutPlayerCharacter* DeadPlayer);

	// 화톳불 상호작용 시 외부에서 호출. CurrentCheckpointActor 갱신.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	virtual void HandleCheckpoint(AActor* BonfireActor);

	// 보스 아레나가 BeginPlay 등에서 자기 등록. 전멸/체크포인트 복귀 시 ResetArena 호출 대상.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	void RegisterArena(TScriptInterface<IBlackoutArenaResettableInterface> Arena);
	
	// ClientTravel URL SessionId DedicatedSessionSubsystem에 위임
	// 데디만 사용
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
	/** 현재 PS-> SelectdClassTag 기준으로 Pawn 재스폰 + Possess. ShelterPrep 캐릭터 변경에서 사용 */
	void RespawnPlayerWithSelectedClass(APlayerController* InController);
	
	UPROPERTY(EditDefaultsOnly, Category="Blackout|Battle|Players")
	TArray<TSubclassOf<APawn>> PlayerClassPool;

	
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

protected:
	// 플레이어 접속 시 전투 진입 자원 초기화 정책 적용 (LobbyToBattle).
	virtual void OnPlayerJoined(APlayerController* NewPlayer) override;
	
	virtual void OnPlayerLeft(AController* Exiting) override;

	// 전원 Ready 시 현재 쉘터 페이즈 → 해당 보스 전투로 전이 + 게이트 Open.
	virtual void OnAllPlayersReady() override;

	// GameState 생성 직후 초기 상태를 WaitingForPlayers 로 세팅.
	virtual void InitGameState() override;

	// 매치 시작 후 신규 접속 거부 (WaitingForPlayers 가 아니면 난입 차단).
	virtual void PreLogin(const FString& Options, const FString& Address,
		const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// 마지막으로 활성화된 화톳불. 파티 전멸 시 이 액터 위치로 복귀.
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Battle")
	TObjectPtr<AActor> CurrentCheckpointActor;

	// 현재 진행 중인 보스 아레나. 전멸/체크포인트 복귀 시 결정적 리셋 대상 (없으면 null).
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Battle")
	TScriptInterface<IBlackoutArenaResettableInterface> CurrentArena;

	// [테스트 전용] true 면 4인 충족 시 클래스선택/Ready UX 생략 즉시 전투 진입.
	// 출시 빌드 기본 false — 활성 시 경고 로그로 silent ship 방지.
	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly , Category = "Blackout|Battle|Demo")
	bool bAutoStartOnFull = false;
	
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
};
