#include "BlackoutPlayerController.h"

#include "BlackoutBattleGameMode.h"
#include "BlackoutGameMode.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutPlayerState.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "BlackoutLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "BlackoutGameState.h"
#include "UI/BlackoutHUD.h"
#include "Data/BOCharacterRoster.h"
#include "UI/BlackoutClassSelectWidget.h"
#include "UI/BlackoutClassSelectWidgetController.h"

void ABlackoutPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	TryInitHUD();

	// 모든 possess path(매칭 / open 콘솔 / 미래 우회 경로) 안전망 — possess 도착 시 게임 모드로 복구.
	// MainMenuGameMode 가 SetInputModeUIOnly 로 viewport 를 잠가둔 잔재 해소.
	// 표준 API 사용으로 PauseMenu 등 BP 측 InputMode 변경 흐름과 충돌 없음.
	if (IsLocalPlayerController())
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void ABlackoutPlayerController::CloseClassSelectUI()
{
	if (!ClassSelectWidget && !ClassSelectController)
	{
		return;
	}
	
	if (ClassSelectWidget)
	{
		ClassSelectWidget->RemoveFromParent();
		ClassSelectWidget = nullptr;
	}
	
	if (ClassSelectController)
	{
		ClassSelectController->OnSelectionConfirmed.RemoveDynamic(this, &ABlackoutPlayerController::HandleClassSelectionConfirmed);
		ClassSelectController = nullptr;
	}
	
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
		{
			if (ClassSelectMappingContext)
			{
				Sub->RemoveMappingContext(ClassSelectMappingContext);
			}
			if (DefaultMappingContext)
			{
				Sub->AddMappingContext(DefaultMappingContext,0);
			}
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
	if (!ClassTag.IsValid())
	{
		return;
	}
	ABlackoutPlayerState* PS = GetPlayerState<ABlackoutPlayerState>();
	if (!PS)
	{
		return;
	}
	// ShelterPrep 외 상태에서의 호출 차단
	const ABlackoutGameState* GS = GetWorld()->GetGameState<ABlackoutGameState>();
	if (!GS || GS->CurrentMatchState != EBlackoutMatchState::ShelterPrep)
	{
		BO_LOG_NET(Warning, "Server_SelectClass: ShelterPrep 외 상태 호출 무시 (%s)", *GetName());
		return;
	}
	
	// 같은 클래스 재선택 무시
	if (PS->SelectedClassTag.MatchesTagExact(ClassTag))
	{
		return;
	}
	
	PS->SelectedClassTag =ClassTag;
	BO_LOG_NET(Log, "Server_SelectClass: %s → %s", *GetName(), *ClassTag.ToString());
	
	if (ABlackoutBattleGameMode* GM = GetWorld()->GetAuthGameMode<ABlackoutBattleGameMode>())
	{
		GM->RespawnPlayerWithSelectedClass(this);
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
	// ChangeState(NAME_Spectating)을 호출하면 엔진이 기본 SpectatorPawn으로 Possess를 옮겨
	// 사망한 캐릭터 참조와 입력 컨텍스트가 어긋납니다. 본 프로젝트는 사망 캐릭터를 그대로 보유하면서
	// ViewTarget만 옮기는 방식이므로 상태 전환은 호출하지 않습니다.
	SetSpectatorInputContextActive(true);
	BO_LOG_CORE(Log, "EnterSpectatorMode: %s", *GetName());
}

void ABlackoutPlayerController::ExitSpectatorMode()
{
	SetSpectatorInputContextActive(false);

	if (APawn* ControlledPawn = GetPawn())
	{
		SetViewTargetWithBlend(ControlledPawn, 0.15f);
	}

	BO_LOG_CORE(Log, "ExitSpectatorMode: %s", *GetName());
}

void ABlackoutPlayerController::SetSpectatorInputContextActive(bool bActive)
{
	if (!IsLocalPlayerController() || !SpectatorMappingContext)
	{
		return;
	}

	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (!InputSubsystem)
	{
		return;
	}

	if (bActive)
	{
		// 기본 IMC보다 우선순위를 높여(1) A/D 키 매핑이 MoveAction을 가리지 않도록 합니다.
		InputSubsystem->AddMappingContext(SpectatorMappingContext, 1);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(SpectatorMappingContext);
	}
}

void ABlackoutPlayerController::OnSpectatePrevPressed()
{
	Server_CycleSpectateTarget(-1);
}

void ABlackoutPlayerController::OnSpectateNextPressed()
{
	Server_CycleSpectateTarget(+1);
}

void ABlackoutPlayerController::Server_CycleSpectateTarget_Implementation(int32 Direction)
{
	if (ABlackoutBattleGameMode* BattleGameMode =
		GetWorld() ? GetWorld()->GetAuthGameMode<ABlackoutBattleGameMode>() : nullptr)
	{
		BattleGameMode->CycleSpectateTargetForSpectator(this, Direction);
	}
}

void ABlackoutPlayerController::Client_SetSpectateTarget_Implementation(AActor* TargetActor, float BlendTime)
{
	if (!TargetActor)
	{
		BO_LOG_CORE(Warning, "Client_SetSpectateTarget 실패: TargetActor가 비어 있음 Controller=%s", *GetNameSafe(this));
		return;
	}

	EnterSpectatorMode();
	SetViewTargetWithBlend(TargetActor, FMath::Max(0.0f, BlendTime));
	BO_LOG_CORE(Log,
		"Client_SetSpectateTarget: Controller=%s Target=%s",
		*GetNameSafe(this),
		*GetNameSafe(TargetActor));
}

void ABlackoutPlayerController::Client_ReturnToOwnPawnView_Implementation(float BlendTime)
{
	// EnterSpectatorMode와 마찬가지로 상태 전환은 사용하지 않습니다. ViewTarget만 본인 폰으로 복귀합니다.
	SetSpectatorInputContextActive(false);

	if (APawn* ControlledPawn = GetPawn())
	{
		SetViewTargetWithBlend(ControlledPawn, FMath::Max(0.0f, BlendTime));
	}
	else
	{
		BO_LOG_CORE(Warning, "Client_ReturnToOwnPawnView 실패: Pawn이 비어 있음 Controller=%s", *GetNameSafe(this));
	}

	BO_LOG_CORE(Log, "Client_ReturnToOwnPawnView: %s", *GetNameSafe(this));
}

void ABlackoutPlayerController::Client_OpenClassSelectUI_Implementation()
{
	if (!ClassSelectWidgetClass || !CharacterRoster)
	{
		BO_LOG_CORE(Warning, "OpenClassSelectUI: WidgetClass 또는 CharacterRoster 미설정 — BP_PlayerController 확인");
		return;
	}
	
	if (ClassSelectWidget)
	{
		return;
	}
	
	ClassSelectController = NewObject<UBlackoutClassSelectWidgetController>(this);
	if (!ClassSelectController || !ClassSelectController->Initialize(this,CharacterRoster))
	{
		ClassSelectController = nullptr;
		return;
	}
	
	ClassSelectController -> OnSelectionConfirmed.AddDynamic(
		this , &ABlackoutPlayerController::HandleClassSelectionConfirmed);
	
	ClassSelectWidget = CreateWidget<UBlackoutClassSelectWidget>(this , ClassSelectWidgetClass);
	if (!ClassSelectWidget)
	{
		ClassSelectController = nullptr;
		return;
	}
	ClassSelectWidget->SetWidgetController(ClassSelectController);
	ClassSelectWidget->AddToViewport();
	
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
		{
			if (DefaultMappingContext)
			{
				Sub->RemoveMappingContext(DefaultMappingContext);
			}
			if (ClassSelectMappingContext)
			{
				Sub->AddMappingContext(ClassSelectMappingContext,0);
			}
		}
	}
	
	ReceiveOpenClassSelectUI();
}

void ABlackoutPlayerController::Client_ShowDamageNumber_Implementation(float DamageAmount, bool bIsCritical)
{
	ReceiveShowDamageNumber(DamageAmount, bIsCritical);
}

void ABlackoutPlayerController::Client_ShowDamageNumberAtLocation_Implementation(
	float DamageAmount,
	FVector WorldLocation,
	bool bIsCritical)
{
	TryInitHUD();

	if (ABlackoutHUD* BlackoutHUD = Cast<ABlackoutHUD>(GetHUD()))
	{
		if (BlackoutHUD->ShowDamageNumberAtWorldLocation(DamageAmount, WorldLocation, bIsCritical))
		{
			return;
		}
	}

	// HUD가 아직 준비되지 않았을 때도 최소한의 표시 경로는 유지합니다.
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

	if (SpectatePrevAction)
	{
		EnhancedInputComponent->BindAction(SpectatePrevAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnSpectatePrevPressed);
	}

	if (SpectateNextAction)
	{
		EnhancedInputComponent->BindAction(SpectateNextAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnSpectateNextPressed);
	}
	
	if (ClassSelectNextAction)
	{
		EnhancedInputComponent->BindAction(ClassSelectNextAction , ETriggerEvent::Started ,this , & ABlackoutPlayerController::OnClassSelectNextPressed);
	}
	
	if (ClassSelectPrevAction)
	{
		EnhancedInputComponent->BindAction(ClassSelectPrevAction ,ETriggerEvent::Started ,this , & ABlackoutPlayerController::OnClassSelectPrevPressed );
	}
	
	if (ClassSelectConfirmAction)
	{
		EnhancedInputComponent->BindAction(ClassSelectConfirmAction,ETriggerEvent::Started,this, &ABlackoutPlayerController::OnClassSelectConfirmPressed);
	}
	
	if (ClassSelectCancelAction)
	{
		EnhancedInputComponent->BindAction(ClassSelectCancelAction ,ETriggerEvent::Started ,this , & ABlackoutPlayerController::OnClassSelectCancelPressed);
	}
}

void ABlackoutPlayerController::OnClassSelectNextPressed()
{
	if (ClassSelectWidget)
	{
		ClassSelectWidget->RequestNavigateNext();
	}
}

void ABlackoutPlayerController::OnClassSelectPrevPressed()
{
	if (ClassSelectWidget)
	{
		ClassSelectWidget->RequestNavigatePrevious();
	}
}

void ABlackoutPlayerController::OnClassSelectConfirmPressed()
{
	if (ClassSelectWidget)
	{
		ClassSelectWidget->RequestConfirm();
	}
}

void ABlackoutPlayerController::OnClassSelectCancelPressed()
{
	CloseClassSelectUI();
}

void ABlackoutPlayerController::HandleClassSelectionConfirmed()
{
	CloseClassSelectUI();
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

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(GetPawn()))
	{
		// 부활 대상이 없는 경우에만 픽업/오브젝트 상호작용을 먼저 시도합니다.
		if (!PlayerCharacter->HasNearbyReviveTarget() && PlayerCharacter->TryInteractWithFocusedActor())
		{
			return;
		}
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
