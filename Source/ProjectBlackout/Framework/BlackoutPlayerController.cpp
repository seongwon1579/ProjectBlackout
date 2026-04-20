#include "BlackoutPlayerController.h"
#include "BlackoutPlayerState.h"
#include "BlackoutLog.h"

void ABlackoutPlayerController::Server_SelectClass_Implementation(FGameplayTag ClassTag)
{
	if (ABlackoutPlayerState* PS = GetPlayerState<ABlackoutPlayerState>())
	{
		PS->SelectedClassTag = ClassTag;
		BO_LOG_NET(Log, "Server_SelectClass: %s → %s", *GetName(), *ClassTag.ToString());
	}
}

void ABlackoutPlayerController::EnterSpectatorMode()
{
	ChangeState(NAME_Spectating);
	BO_LOG_CORE(Log, "EnterSpectatorMode: %s", *GetName());
}

void ABlackoutPlayerController::Client_OpenClassSelectUI_Implementation()
{
	ReceiveOpenClassSelectUI();
}

void ABlackoutPlayerController::Client_ShowDamageNumber_Implementation(float DamageAmount, bool bIsCritical)
{
	ReceiveShowDamageNumber(DamageAmount, bIsCritical);
}
