#include "BlackoutPlayerController.h"

#include "BlackoutGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutLog.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

void ABlackoutPlayerController::Server_SelectClass_Implementation(FGameplayTag ClassTag)
{
	if (ABlackoutPlayerState* PS = GetPlayerState<ABlackoutPlayerState>())
	{
		PS->SelectedClassTag = ClassTag;
		BO_LOG_NET(Log, "Server_SelectClass: %s → %s", *GetName(), *ClassTag.ToString());
	}
}

void ABlackoutPlayerController::Server_SetReady_Implementation(bool bNewReady)
{
	ABlackoutPlayerState* PS = GetPlayerState<ABlackoutPlayerState>();
	if (!PS)
	{
		return;
	}
	PS->bIsReady = bNewReady;
	BO_LOG_NET(Log , "Server_SetReady:%s -> %s",*GetName(), bNewReady ? TEXT("Ready") : TEXT("NotReady"));
	
	// Lobby / Battle 공통. 부모 GameMode 의 NotifyReadyChanged 가 자식의 OnAllPlayersReady 훅을 호출.
	if (ABlackoutGameMode* Mode = GetWorld()->GetAuthGameMode<ABlackoutGameMode>())
	{
		Mode->NotifyReadyChanged();
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

#pragma region InputSetup

void ABlackoutPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalPlayerController())
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (DefaultMappingContext)
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		if (MouseLookMappingContext)
		{
			InputSubsystem->AddMappingContext(MouseLookMappingContext, 0);
		}
	}
}

#pragma endregion 