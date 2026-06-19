// ─── 구현 내역 ───────────────────────
//  - 허혁: 개발용 치트 매니저 — 매치 상태/무한 체력·스태미나·탄약·스턴 게이지 디버그 Exec 진입점
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "BlackoutCheatManager.generated.h"

class ABlackoutPlayerController;

/**
 * 프로젝트 전용 치트 매니저입니다.
 * Exec 진입점은 여기로 모으고, 실제 서버 적용은 PlayerController의 공용 치트 디스패처를 통해 수행합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec, Category = "Blackout|Cheat")
	void BO_SetMatchState(const FString& NewStateStr);

	UFUNCTION(Exec, Category = "Blackout|Cheat")
	void BO_InfiniteHealth(bool bEnabled = true);

	UFUNCTION(Exec, Category = "Blackout|Cheat")
	void BO_InfiniteStamina(bool bEnabled = true);

	UFUNCTION(Exec, Category = "Blackout|Cheat")
	void BO_InfiniteAmmo(bool bEnabled = true);

	UFUNCTION(Exec, Category = "Blackout|Cheat")
	void BO_DebugStunGauge(bool bEnabled = true);

private:
	ABlackoutPlayerController* GetBlackoutPlayerController() const;
	void ExecuteOrForwardCheatCommand(const FString& CheatCommand);
};
