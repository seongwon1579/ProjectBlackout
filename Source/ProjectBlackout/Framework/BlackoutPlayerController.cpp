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
#include "UI/BlackoutMainMenuWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "InputCoreTypes.h"

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
	
	// seamless travel 도착 후 폰빙의시 화면 복귀
	if (bScreenFadePending)
	{
		bScreenFadePending = false;
		StartScreenFadeIn();
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

void ABlackoutPlayerController::OpenPlayerMenu()
{
	if (!IsLocalPlayerController() || ActivePlayerMenuWidget)
	{
		return;
	}

	if (!PlayerMenuWidgetClass)
	{
		BO_LOG_CORE(Warning, "OpenPlayerMenu 실패: PlayerMenuWidgetClass가 설정되지 않았습니다.");
		return;
	}

	ActivePlayerMenuWidget = CreateWidget<UBlackoutMainMenuWidget>(this, PlayerMenuWidgetClass);
	if (!ActivePlayerMenuWidget)
	{
		BO_LOG_CORE(Warning, "OpenPlayerMenu 실패: 인게임 메뉴 위젯 생성에 실패했습니다.");
		return;
	}

	// 인게임 ESC 메뉴 전용 모드로 전환하여 불필요한 메인메뉴 버튼을 숨깁니다.
	ActivePlayerMenuWidget->bUseAsInGameMenu = true;
	ActivePlayerMenuWidget->OnMenuClosed.AddDynamic(this, &ABlackoutPlayerController::HandlePlayerMenuClosed);
	ActivePlayerMenuWidget->AddToViewport(110);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ActivePlayerMenuWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	ActivePlayerMenuWidget->SetKeyboardFocus();
}

void ABlackoutPlayerController::ClosePlayerMenu()
{
	if (!ActivePlayerMenuWidget)
	{
		return;
	}

	ActivePlayerMenuWidget->OnMenuClosed.RemoveDynamic(this, &ABlackoutPlayerController::HandlePlayerMenuClosed);
	ActivePlayerMenuWidget->RemoveFromParent();
	ActivePlayerMenuWidget = nullptr;

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void ABlackoutPlayerController::Client_StartScreenFadeOut_Implementation(
	FLinearColor FadeColor)
{
	if (!PlayerCameraManager)
	{
		return;
	}
	LastFadeColor = FadeColor;
	bScreenFadePending = true;
	
	PlayerCameraManager->StartCameraFade(0.0f ,1.0f ,ScreenFadeDuration , FadeColor ,false ,true);
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
	
	if (ABlackoutGameMode* GM = GetWorld()->GetAuthGameMode<ABlackoutGameMode>())
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
	PS->SetReadyState(bNewReady);
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
	if (!ClassSelectWidgetClass)
	{
		BO_LOG_CORE(Warning, "OpenClassSelectUI: WidgetClass 미설정— BP_PlayerController 확인");
		return;
	}
	
	const ABlackoutGameState* GS = GetWorld()->GetGameState<ABlackoutGameState>();
	const UBOCharacterRoster* CharacterRoster = GS ? GS->CharacterRoster :nullptr;
	
	if (!CharacterRoster)
	{
		// GameState OnRep(TryOpenLocalClassSelectUI) 시점에 호출되므로 정상 흐름에선 도달 X. 방어 가드.
		BO_LOG_CORE(Warning, "OpenClassSelectUI: CharacterRoster 미수신 — BP_BlackoutGameState 확인");
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

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ABlackoutPlayerController::OnMenuTogglePressed);
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

	if (RequestSurrenderAction)
	{
		EnhancedInputComponent->BindAction(RequestSurrenderAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnRequestSurrenderPressed);
	}

	if (VoteYesAction)
	{
		EnhancedInputComponent->BindAction(VoteYesAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnVoteYesPressed);
	}

	if (VoteNoAction)
	{
		EnhancedInputComponent->BindAction(VoteNoAction, ETriggerEvent::Started, this, &ABlackoutPlayerController::OnVoteNoPressed);
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

void ABlackoutPlayerController::OnMenuTogglePressed()
{
	// 캐릭터 선택 UI가 열려 있으면 메뉴보다 먼저 닫기 동작을 우선합니다.
	if (ClassSelectWidget || ClassSelectController)
	{
		CloseClassSelectUI();
		return;
	}

	if (ActivePlayerMenuWidget)
	{
		ClosePlayerMenu();
		return;
	}

	OpenPlayerMenu();
}

void ABlackoutPlayerController::HandleClassSelectionConfirmed()
{
	CloseClassSelectUI();
}

void ABlackoutPlayerController::HandlePlayerMenuClosed()
{
	ClosePlayerMenu();
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
		PlayerCharacter->Server_RequestDebugSelfDamage(30.f);
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

void ABlackoutPlayerController::Server_RequestSurrenderVote_Implementation()
{
	if (ABlackoutBattleGameMode* BattleGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ABlackoutBattleGameMode>() : nullptr)
	{
		BattleGameMode->StartSurrenderVote(this);
	}
}

void ABlackoutPlayerController::Server_CastSurrenderVote_Implementation(bool bAgree)
{
	if (ABlackoutBattleGameMode* BattleGameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ABlackoutBattleGameMode>() : nullptr)
	{
		BattleGameMode->CastSurrenderVote(this, bAgree);
	}
}

void ABlackoutPlayerController::Client_SetSurrenderInputContextActive_Implementation(bool bActive)
{
	if (!IsLocalPlayerController() || !SurrenderVoteMappingContext)
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
		// 높은 우선순위(2)로 매핑 컨텍스트를 푸시하여 투표 조작이 최우선이 되도록 합니다.
		InputSubsystem->AddMappingContext(SurrenderVoteMappingContext, 2);
	}
	else
	{
		InputSubsystem->RemoveMappingContext(SurrenderVoteMappingContext);
	}
}

void ABlackoutPlayerController::OnVoteYesPressed()
{
	Server_CastSurrenderVote(true);
}

void ABlackoutPlayerController::OnVoteNoPressed()
{
	Server_CastSurrenderVote(false);
}

void ABlackoutPlayerController::OnRequestSurrenderPressed()
{
	// 서버에 항복 투표 발의(요청)를 보냅니다.
	Server_RequestSurrenderVote();
}

#pragma endregion 
void ABlackoutPlayerController::StartScreenFadeIn()
{
	if (!PlayerCameraManager)
	{
		return;
	}
	PlayerCameraManager->StartCameraFade(1.0f, 0.0f , ScreenFadeDuration , LastFadeColor , false , false);
}

void ABlackoutPlayerController::BO_SetMatchState(const FString& NewStateStr)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
	FString TargetState = NewStateStr.ToLower().TrimStartAndEnd();
	EBlackoutMatchState SelectedState = EBlackoutMatchState::WaitingForPlayers;
	bool bIsValid = false;

	if (TargetState.Equals(TEXT("inlobby")) || TargetState.Equals(TEXT("lobby")))
	{
		SelectedState = EBlackoutMatchState::InLobby;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("starting")) || TargetState.Equals(TEXT("start")))
	{
		SelectedState = EBlackoutMatchState::Starting;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("incombatready")) || TargetState.Equals(TEXT("ready")))
	{
		SelectedState = EBlackoutMatchState::InCombatReady;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("incombat")) || TargetState.Equals(TEXT("combat")) || TargetState.Equals(TEXT("c")))
	{
		SelectedState = EBlackoutMatchState::InCombat;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("ended")) || TargetState.Equals(TEXT("end")) || TargetState.Equals(TEXT("e")))
	{
		SelectedState = EBlackoutMatchState::Ended;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("waitingforplayers")) || TargetState.Equals(TEXT("waiting")) || TargetState.Equals(TEXT("wait")) || TargetState.Equals(TEXT("w")))
	{
		SelectedState = EBlackoutMatchState::WaitingForPlayers;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("shelterprep")) || TargetState.Equals(TEXT("prep")) || TargetState.Equals(TEXT("s1")))
	{
		SelectedState = EBlackoutMatchState::ShelterPrep;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("midbosscombat")) || TargetState.Equals(TEXT("midboss")) || TargetState.Equals(TEXT("mid")) || TargetState.Equals(TEXT("m")))
	{
		SelectedState = EBlackoutMatchState::MidBossCombat;
		bIsValid = true;
	}
	else if (TargetState.Equals(TEXT("mainbosscombat")) || TargetState.Equals(TEXT("mainboss")) || TargetState.Equals(TEXT("main")) || TargetState.Equals(TEXT("b")))
	{
		SelectedState = EBlackoutMatchState::MainBossCombat;
		bIsValid = true;
	}

	if (bIsValid)
	{
		Server_SetMatchStateCheat(SelectedState);
	}
	else
	{
		BO_LOG_CORE(Warning, TEXT("알 수 없는 매치 상태 치트 문자열입니다: %s"), *NewStateStr);
	}
#else
	BO_LOG_CORE(Warning, TEXT("개발 빌드가 아닌 환경에서는 매치 상태 치트 명령을 사용할 수 없습니다: %s"), *NewStateStr);
#endif
}

void ABlackoutPlayerController::Server_SetMatchStateCheat_Implementation(EBlackoutMatchState NewState)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
	if (ABlackoutGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr)
	{
		GS->SetMatchState(NewState);
		BO_LOG_NET(Log, TEXT("치트 명령어로 매치 상태를 강제 전환했습니다: %s"), *UEnum::GetValueAsString(NewState));
	}
#else
	BO_LOG_NET(Warning, TEXT("개발 빌드가 아닌 환경에서 매치 상태 치트 RPC가 차단되었습니다: %s"), *UEnum::GetValueAsString(NewState));
#endif
}

bool ABlackoutPlayerController::Server_SetMatchStateCheat_Validate(EBlackoutMatchState NewState)
{
#if WITH_EDITOR || UE_BUILD_DEVELOPMENT
	return true;
#else
	return false;
#endif
}
