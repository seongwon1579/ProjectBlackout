#include "Framework/BlackoutCheatManager.h"

#include "Core/BlackoutLog.h"
#include "Framework/BlackoutPlayerController.h"

ABlackoutPlayerController* UBlackoutCheatManager::GetBlackoutPlayerController() const
{
	return Cast<ABlackoutPlayerController>(GetOuter());
}

void UBlackoutCheatManager::ExecuteOrForwardCheatCommand(const FString& CheatCommand)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
	ABlackoutPlayerController* BlackoutPlayerController = GetBlackoutPlayerController();
	if (!BlackoutPlayerController)
	{
		BO_LOG_CORE(Warning, TEXT("치트 실행 실패: BlackoutPlayerController를 찾지 못했습니다. Command=%s"), *CheatCommand);
		return;
	}

	if (BlackoutPlayerController->HasAuthority())
	{
		BlackoutPlayerController->ExecuteCheatCommandLocally(CheatCommand);
		return;
	}

	BlackoutPlayerController->Server_RunCheatCommand(CheatCommand);
#else
	BO_LOG_CORE(Warning, TEXT("개발 빌드가 아닌 환경에서는 치트를 사용할 수 없습니다: %s"), *CheatCommand);
#endif
}

void UBlackoutCheatManager::BO_SetMatchState(const FString& NewStateStr)
{
	ExecuteOrForwardCheatCommand(FString::Printf(TEXT("BO_SetMatchState %s"), *NewStateStr));
}

void UBlackoutCheatManager::BO_InfiniteHealth(bool bEnabled)
{
	ExecuteOrForwardCheatCommand(FString::Printf(TEXT("BO_InfiniteHealth %d"), bEnabled ? 1 : 0));
}

void UBlackoutCheatManager::BO_InfiniteStamina(bool bEnabled)
{
	ExecuteOrForwardCheatCommand(FString::Printf(TEXT("BO_InfiniteStamina %d"), bEnabled ? 1 : 0));
}

void UBlackoutCheatManager::BO_InfiniteAmmo(bool bEnabled)
{
	ExecuteOrForwardCheatCommand(FString::Printf(TEXT("BO_InfiniteAmmo %d"), bEnabled ? 1 : 0));
}

void UBlackoutCheatManager::BO_DebugStunGauge(bool bEnabled)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
	ABlackoutPlayerController* BlackoutPlayerController = GetBlackoutPlayerController();
	if (!BlackoutPlayerController)
	{
		BO_LOG_CORE(Warning, TEXT("스턴 게이지 디버그 표시 실패: BlackoutPlayerController를 찾지 못했습니다."));
		return;
	}

	BlackoutPlayerController->SetStunGaugeDebugEnabled(bEnabled);
#else
	BO_LOG_CORE(Warning, TEXT("개발 빌드가 아닌 환경에서는 스턴 게이지 디버그 표시를 사용할 수 없습니다."));
#endif
}
