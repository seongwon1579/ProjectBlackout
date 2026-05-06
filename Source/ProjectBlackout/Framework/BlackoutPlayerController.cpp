#include "BlackoutPlayerController.h"

#include "BlackoutGameMode.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutPlayerState.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "BlackoutLog.h"
#include "EnhancedInputComponent.h"
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

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	if (FireAction)
	{
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnFirePressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ABlackoutPlayerController::OnFireReleased);
	}

	if (AimAction)
	{
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnAimPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ABlackoutPlayerController::OnAimReleased);
	}

	if (ReloadAction)
	{
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnReloadPressed);
	}

	if (SwapWeaponAction)
	{
		EnhancedInputComponent->BindAction(SwapWeaponAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnSwapWeaponPressed);
	}

	if (DodgeAction)
	{
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnDodgePressed);
	}

	if (SprintAction)
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnSprintPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ABlackoutPlayerController::OnSprintReleased);
	}

	if (DebugSelfDamageAction)
	{
		EnhancedInputComponent->BindAction(DebugSelfDamageAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnDebugSelfDamagePressed);
	}
	
	if (InteractAction)
	{
		EnhancedInputComponent->BindAction(InteractAction , ETriggerEvent::Started , this , &ABlackoutPlayerController::OnInteractPressed);
		EnhancedInputComponent->BindAction(InteractAction , ETriggerEvent::Completed , this , &ABlackoutPlayerController::OnInteractReleased);
		EnhancedInputComponent->BindAction(InteractAction , ETriggerEvent::Canceled , this , &ABlackoutPlayerController::OnInteractReleased);
		
	}
	
}

void ABlackoutPlayerController::OnFirePressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	if (UBlackoutCombatComponent* CombatComponent = GetBlackoutCombatComponent())
	{
		CombatComponent->HandlePrimaryActionPressed();
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Fire);
}

void ABlackoutPlayerController::OnFireReleased()
{
	if (UBlackoutCombatComponent* CombatComponent = GetBlackoutCombatComponent())
	{
		CombatComponent->HandlePrimaryActionReleased();
		return;
	}

	HandleAbilityInputReleased(EBlackoutAbilityInputID::Fire);
}

void ABlackoutPlayerController::OnAimPressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	if (UBlackoutCombatComponent* CombatComponent = GetBlackoutCombatComponent())
	{
		CombatComponent->StartAim();
	}
}

void ABlackoutPlayerController::OnAimReleased()
{
	if (UBlackoutCombatComponent* CombatComponent = GetBlackoutCombatComponent())
	{
		CombatComponent->StopAim();
	}
}

void ABlackoutPlayerController::OnReloadPressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Reload);
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Reload);
}

void ABlackoutPlayerController::OnSwapWeaponPressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	if (UBlackoutCombatComponent* CombatComponent = GetBlackoutCombatComponent())
	{
		CombatComponent->SwapWeapon();
	}
}

void ABlackoutPlayerController::OnDodgePressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetPawn()))
	{
		const FVector2D DodgeInput = PlayerCharacter->GetCachedMoveInput();

		// 로컬 예측용
		PlayerCharacter->SetPendingDodgeInput(DodgeInput);

		// 서버 권한용
		PlayerCharacter->Server_SetPendingDodgeInput(DodgeInput);
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Dodge);
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Dodge);
}

void ABlackoutPlayerController::OnSprintPressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Sprint);
}

void ABlackoutPlayerController::OnSprintReleased()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Sprint);
}

void ABlackoutPlayerController::OnDebugSelfDamagePressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->Server_RequestDebugSelfDamage(10.f);
	}
}

void ABlackoutPlayerController::OnInteractPressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Interact);

}

void ABlackoutPlayerController::OnInteractReleased()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Interact);

}

bool ABlackoutPlayerController::IsHitReactInputBlocked() const
{
	const ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetPawn());
	return PlayerCharacter
		&& (PlayerCharacter->IsHitReactMontagePlaying()
			|| PlayerCharacter->IsDowned()
			|| PlayerCharacter->IsDead());
}

void ABlackoutPlayerController::HandleAbilityInputPressed(EBlackoutAbilityInputID InputID)
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = GetBlackoutAbilitySystemComponent())
	{
		AbilitySystemComponent->HandleAbilityInputPressed(InputID);
	}
}

void ABlackoutPlayerController::HandleAbilityInputReleased(EBlackoutAbilityInputID InputID)
{
	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = GetBlackoutAbilitySystemComponent())
	{
		AbilitySystemComponent->HandleAbilityInputReleased(InputID);
	}
}

UBlackoutAbilitySystemComponent* ABlackoutPlayerController::GetBlackoutAbilitySystemComponent() const
{
	const ABlackoutPlayerState* BlackoutPlayerState = GetPlayerState<ABlackoutPlayerState>();
	return BlackoutPlayerState ? BlackoutPlayerState->GetBlackoutAbilitySystemComponent() : nullptr;
}

UBlackoutCombatComponent* ABlackoutPlayerController::GetBlackoutCombatComponent() const
{
	const ABlackoutPlayerCharacter* BlackoutPlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetPawn());
	return BlackoutPlayerCharacter ? BlackoutPlayerCharacter->GetCombatComponent() : nullptr;
}

#pragma endregion 
