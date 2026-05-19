#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "Interfaces/BlackoutArenaResettable.h"
#include "BlackoutBattleGameMode.generated.h"

enum class EBlackoutMatchEndReason : uint8;
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
	
	// 생존자 0명 감지 시 외부에서 호출. 현재 체크포인트로 전원 복귀 트리거.
	virtual void HandlePartyWipe() override;

	// 화톳불 상호작용 시 외부에서 호출. CurrentCheckpointActor 갱신.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	virtual void HandleCheckpoint(AActor* BonfireActor);

	// 보스 아레나가 BeginPlay 등에서 자기 등록. 전멸/체크포인트 복귀 시 ResetArena 호출 대상.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	void RegisterArena(TScriptInterface<IBlackoutArenaResettableInterface> Arena);
	
	// ClientTravel URL SessionId DedicatedSessionSubsystem에 위임
	// 데디만 사용
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
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
	
private:
	int32 NextPlayerClassIndex =0;

	// 엔진이 ChoosePlayerStart / SpawnDefaultPawnFor / FindPlayerStart 경로에서
	// GetDefaultPawnClassForController_Implementation을 한 로그인당 여러 번 호출함.
	// 컨트롤러별 캐시로 동일 컨트롤러에 같은 클래스 반환 → 라운드로빈 카운터 한 번만 증가.
	UPROPERTY()
	TMap<TObjectPtr<AController>, TSubclassOf<APawn>> ControllerToClass;
};
