#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "BlackoutBattleGameMode.generated.h"

enum class EBlackoutMatchEndReason : uint8;
class ABlackoutPlayerCharacter;

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
	
	// 생존자 0명 감지 시 외부에서 호출. 현재 체크포인트로 전원 복귀 트리거.
	virtual void HandlePartyWipe() override;

	// 플레이어가 다운 타이머 만료로 완전 사망했을 때 호출. 생존자 0명 여부를 평가한다.
	void NotifyPlayerFullyDead(ABlackoutPlayerCharacter* DeadPlayer);

	// 화톳불 상호작용 시 외부에서 호출. CurrentCheckpointActor 갱신.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	virtual void HandleCheckpoint(AActor* BonfireActor);
	
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

	// 전원 Ready 시 보스 활성화 훅 + InCombat 전환.
	virtual void OnAllPlayersReady() override;

	// 마지막으로 활성화된 화톳불. 파티 전멸 시 이 액터 위치로 복귀.
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Battle")
	TObjectPtr<AActor> CurrentCheckpointActor;
	
	// 시연용
	UPROPERTY(EditDefaultsOnly ,BlueprintReadOnly , Category = "Blackout|Battle|Demo")
	bool bAutoStartOnFull = true;
	
private:
	void EvaluatePartyWipe();

	int32 NextPlayerClassIndex =0;

	// 엔진이 ChoosePlayerStart / SpawnDefaultPawnFor / FindPlayerStart 경로에서
	// GetDefaultPawnClassForController_Implementation을 한 로그인당 여러 번 호출함.
	// 컨트롤러별 캐시로 동일 컨트롤러에 같은 클래스 반환 → 라운드로빈 카운터 한 번만 증가.
	UPROPERTY()
	TMap<TObjectPtr<AController>, TSubclassOf<APawn>> ControllerToClass;
};
