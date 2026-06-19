// ─── 구현 내역 ───────────────────────
//  - 최승현: 로비 GameMode — 전원 Ready 집계 후 보스맵 ServerTravel·복귀 시 선택 클래스 spawn·로비 도착 회복/ShelterPrep
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutGameMode.h"
#include "UObject/SoftObjectPath.h"
#include "BlackoutLobbyGameMode.generated.h"

UCLASS()
class PROJECTBLACKOUT_API ABlackoutLobbyGameMode : public ABlackoutGameMode
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|Lobby")
	virtual void StartBattle();

	// [테스트 전용] 정원/Ready 무시 강제 ServerTravel. seamless 검증용 (2인 PIE 등).
	UFUNCTION(Exec)
	void BO_ForceStartBattle();

	// 복귀 로비에서 보존된 SelectedClassTag 로 spawn (없으면 DefaultPawnClass).
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

protected:
	
	virtual void OnPlayerJoined(APlayerController* NewPlayer) override;

	// 전원 Ready 성립 시 StartBattle 트리거.
	virtual void OnAllPlayersReady() override;

	virtual void OnSeamlessArrival(APlayerController* PC) override;

	// 빈 서버(로비) 복귀 — 매칭 서버에 finish/idle 보고(안 하면 status='playing' 좀비) + 로비 reload.
	virtual void HandleEmptyServerReset() override;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Lobby")
	TArray<FSoftObjectPath> BossStageMapPaths;
	
	// ServerTravel 중복 실행 방지 플래그 , StartBattle 최초 1회만
	UPROPERTY(BlueprintReadOnly , Category = "Blackout|Lobby")
	bool bTravelInitiated = false;

private:
	// 로비 도착 공통 처리 (입장=PostLogin, 복귀=seamless). 회복 + 전원 도착 시 ShelterPrep.
	void HandleLobbyArrival(APlayerController* PC);
	
	// 페이드 아웃 대기후 실제 보스맵 servertravel
	void DoStartBattleTravel();
	
	FTimerHandle FadeTravelTimerHandle;
};
