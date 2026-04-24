#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
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
	
	// 생존자 0명 감지 시 외부에서 호출. 현재 체크포인트로 전원 복귀 트리거.
	virtual void HandlePartyWipe() override;

	// 화톳불 상호작용 시 외부에서 호출. CurrentCheckpointActor 갱신.
	UFUNCTION(BlueprintCallable, Category = "Blackout|Battle")
	virtual void HandleCheckpoint(AActor* BonfireActor);

protected:
	// 플레이어 접속 시 전투 진입 자원 초기화 정책 적용 (LobbyToBattle).
	virtual void OnPlayerJoined(APlayerController* NewPlayer) override;
	
	virtual void OnPlayerLeft(AController* Exiting) override;

	// 전원 Ready 시 보스 활성화 훅 + InCombat 전환.
	virtual void OnAllPlayersReady() override;

	// 마지막으로 활성화된 화톳불. 파티 전멸 시 이 액터 위치로 복귀.
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Battle")
	TObjectPtr<AActor> CurrentCheckpointActor;
};
