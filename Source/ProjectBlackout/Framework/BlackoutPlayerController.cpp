#include "BlackoutPlayerController.h"

#include "BlackoutGameMode.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutPlayerState.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "BlackoutLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/GameViewportClient.h"
#include "InputMappingContext.h"
#include "UI/BlackoutHUD.h"

void ABlackoutPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	TryInitHUD();

	// 안전망: ClientTravel 후 MoviePlayer가 viewport input lock(bIgnoreInput / NoCapture / DoNotLock) 잔존시키는 케이스 강제 해소.
	// 원인 추적은 시연 후 후속 PR에서 — 지금은 입력 살리는 게 우선.
	if (UWorld* World = GetWorld())
	{
		if (UGameViewportClient* Viewport = World->GetGameViewport())
		{
			Viewport->SetIgnoreInput(false);
			Viewport->SetMouseCaptureMode(EMouseCaptureMode::CapturePermanently);
			Viewport->SetMouseLockMode(EMouseLockMode::LockOnCapture);
		}
	}
}

void ABlackoutPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	TryInitHUD();
}

void ABlackoutPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	TryInitHUD();
}

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

	if (UseConsumable1Action)
	{
		EnhancedInputComponent->BindAction(UseConsumable1Action, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnUseConsumable1Pressed);
		EnhancedInputComponent->BindAction(UseConsumable1Action, ETriggerEvent::Completed, this, &ABlackoutPlayerController::OnUseConsumable1Released);
		EnhancedInputComponent->BindAction(UseConsumable1Action, ETriggerEvent::Canceled, this, &ABlackoutPlayerController::OnUseConsumable1Released);
	}

	if (UseConsumable2Action)
	{
		EnhancedInputComponent->BindAction(UseConsumable2Action, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnUseConsumable2Pressed);
		EnhancedInputComponent->BindAction(UseConsumable2Action, ETriggerEvent::Completed, this, &ABlackoutPlayerController::OnUseConsumable2Released);
		EnhancedInputComponent->BindAction(UseConsumable2Action, ETriggerEvent::Canceled, this, &ABlackoutPlayerController::OnUseConsumable2Released);
	}

	if (UseRelicAction)
	{
		EnhancedInputComponent->BindAction(UseRelicAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnUseRelicPressed);
		EnhancedInputComponent->BindAction(UseRelicAction, ETriggerEvent::Completed, this, &ABlackoutPlayerController::OnUseRelicReleased);
		EnhancedInputComponent->BindAction(UseRelicAction, ETriggerEvent::Canceled, this, &ABlackoutPlayerController::OnUseRelicReleased);
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

	HandleAbilityInputPressed(EBlackoutAbilityInputID::Aim);
}

void ABlackoutPlayerController::OnAimReleased()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::Aim);
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

	HandleAbilityInputPressed(EBlackoutAbilityInputID::SwapWeapon);
	HandleAbilityInputReleased(EBlackoutAbilityInputID::SwapWeapon);
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

void ABlackoutPlayerController::OnUseConsumable1Pressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::UseConsumable1);
}

void ABlackoutPlayerController::OnUseConsumable1Released()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::UseConsumable1);
}

void ABlackoutPlayerController::OnUseConsumable2Pressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::UseConsumable2);
}

void ABlackoutPlayerController::OnUseConsumable2Released()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::UseConsumable2);
}

void ABlackoutPlayerController::OnUseRelicPressed()
{
	if (IsHitReactInputBlocked())
	{
		return;
	}

	HandleAbilityInputPressed(EBlackoutAbilityInputID::UseRelic);
}

void ABlackoutPlayerController::OnUseRelicReleased()
{
	HandleAbilityInputReleased(EBlackoutAbilityInputID::UseRelic);
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

void ABlackoutPlayerController::TryInitHUD() const
{
	if (!IsLocalController())
	{
		return;
	}

	ABlackoutHUD* BlackoutHUD = Cast<ABlackoutHUD>(GetHUD());
	if (!BlackoutHUD)
	{
		return;
	}

	BlackoutHUD->InitHUD();
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
