// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutMainMenuWidget.h"

#include "BlackoutLog.h"
#include "BlackoutLoginWidget.h"
#include "BlackoutMatchFlowSubsystem.h"
#include "BlackoutMatchmakingWidget.h"
#include "BlackoutSettingsWidget.h"
#include "Framework/BlackoutMusicSubsystem.h"
#include "Framework/BlackoutMatchmakingSubsystem.h"

#include  "Components/Button.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

void UBlackoutMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(
			this, &UBlackoutMainMenuWidget::HandleLoginClicked);
	}
	if (LogoutButton)
	{
		LogoutButton->OnClicked.AddDynamic(
			this, &UBlackoutMainMenuWidget::HandleLogoutClicked);
	}
	if (StartMatchmakingButton)
	{
		StartMatchmakingButton->OnClicked.AddDynamic(
			this, &UBlackoutMainMenuWidget::HandleStartMatchmakingClicked);
	}
	
	if (ReconnectButton)
	{
		ReconnectButton->OnClicked.AddDynamic(this, &UBlackoutMainMenuWidget::HandleReconnectClicked);
	}
	
	if (SinglePlayButton)
	{
		SinglePlayButton->OnClicked.AddDynamic(
			this, &UBlackoutMainMenuWidget::HandleSinglePlayClicked);
	}

	if (OptionsButton)
	{
		OptionsButton->OnClicked.AddDynamic(
			this, &UBlackoutMainMenuWidget::HandleOptionsClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(
			this, &UBlackoutMainMenuWidget::HandleQuitClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddDynamic(
			this, &UBlackoutMainMenuWidget::HandleBackClicked);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->
			GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->OnMatchmakingError.AddDynamic(
				this, &UBlackoutMainMenuWidget::HandleMatchmakingError);
			
			MatchmakingSubsystem->OnActiveSessionFound.AddDynamic(
				this, &UBlackoutMainMenuWidget::HandleActiveSessionFound);
		}
	}

	// 타이틀 전용 메뉴에서만 메인 메뉴 BGM을 시작하고, 인게임 ESC 메뉴에서는 현재 게임 음악을 유지합니다.
	if (!bUseAsInGameMenu)
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UBlackoutMusicSubsystem* MusicSubsystem = GameInstance->GetSubsystem<UBlackoutMusicSubsystem>())
			{
				MusicSubsystem->PlayMainMenuMusic();
			}
		}
	}

	RefreshForLoginState();
	SetKeyboardFocus();
}

void UBlackoutMainMenuWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->
			GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->OnMatchmakingError.RemoveDynamic(
				this, &UBlackoutMainMenuWidget::HandleMatchmakingError);
			
			MatchmakingSubsystem->OnActiveSessionFound.RemoveDynamic(
				this , &UBlackoutMainMenuWidget::HandleActiveSessionFound);
		}
	}
	if (ActiveLoginWidget)
	{
		ActiveLoginWidget->OnLoginAttemptFinished.RemoveDynamic(
			this, &UBlackoutMainMenuWidget::HandleLoginAttemptFinished);
		ActiveLoginWidget = nullptr;
	}
	if (ActiveMatchmakingWidget)
	{
		ActiveMatchmakingWidget->OnMatchmakingExited.RemoveDynamic(
			this, & UBlackoutMainMenuWidget::HandleMatchmakingWidgetExited);
		ActiveMatchmakingWidget = nullptr;
	}
	if (ActiveSettingsWidget)
	{
		ActiveSettingsWidget->OnSettingsClosed.RemoveDynamic(
			this, &UBlackoutMainMenuWidget::HandleSettingsClosed);
		ActiveSettingsWidget = nullptr;
	}
	
	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(
			this, &UBlackoutMainMenuWidget::HandleBackClicked);
	}
	Super::NativeDestruct();
}

FReply UBlackoutMainMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry,
                                                const FKeyEvent& InKeyEvent)
{
	if (bUseAsInGameMenu && !ActiveSettingsWidget && InKeyEvent.GetKey() ==
		EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UBlackoutMainMenuWidget::HandleLoginClicked()
{
	if (!LoginWidgetClass || ActiveLoginWidget)
	{
		return;
	}
	ActiveLoginWidget = CreateWidget<UBlackoutLoginWidget>(
		GetOwningPlayer(), LoginWidgetClass);
	if (!ActiveLoginWidget)
	{
		return;
	}
	ActiveLoginWidget->OnLoginAttemptFinished.AddDynamic(
		this, &UBlackoutMainMenuWidget::HandleLoginAttemptFinished);
	ActiveLoginWidget->AddToViewport(100);
}

void UBlackoutMainMenuWidget::HandleLogoutClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->
			GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->Logout();
		}
	}
	RefreshForLoginState();
}

void UBlackoutMainMenuWidget::HandleStartMatchmakingClicked()
{
	if (!MatchmakingWidgetClass || ActiveMatchmakingWidget)
	{
		return;
	}
	ActiveMatchmakingWidget = CreateWidget<UBlackoutMatchmakingWidget>(
		GetOwningPlayer(), MatchmakingWidgetClass);
	if (!ActiveMatchmakingWidget)
	{
		return;
	}
	ActiveMatchmakingWidget->OnMatchmakingExited.AddDynamic(
		this, &UBlackoutMainMenuWidget::HandleMatchmakingWidgetExited);
	ActiveMatchmakingWidget->AddToViewport(100);
}

void UBlackoutMainMenuWidget::HandleReconnectClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->TravelToActiveSession();
		}
	}
}

void UBlackoutMainMenuWidget::HandleSinglePlayClicked()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// 정원 1 -> 로비/보스 GameMode InitGame이 MaxPlayer=1로 처리
	if (UBlackoutMatchFlowSubsystem* MatchFlowSubsystem = GameInstance->
		GetSubsystem<UBlackoutMatchFlowSubsystem>())
	{
		MatchFlowSubsystem->SetExpectedPlayers(1);
	}

	if (SinglePlayLobbyMap.IsNull())
	{
		BO_LOG_NET(Warning, "혼자하기: SinglePlayLobbyMap 미지정 — 진입 중단");
		return;
	}

	// listen
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, SinglePlayLobbyMap, true,
	                                           TEXT("listen"));
}

void UBlackoutMainMenuWidget::HandleOptionsClicked()
{
	if (ActiveSettingsWidget)
	{
		return;
	}

	if (!SettingsWidgetClass)
	{
		BO_LOG_CORE(Warning, "옵션 메뉴: SettingsWidgetClass 미지정 — 설정 위젯 생성을 중단합니다.");
		return;
	}

	ActiveSettingsWidget = CreateWidget<UBlackoutSettingsWidget>(
		GetOwningPlayer(), SettingsWidgetClass);
	if (!ActiveSettingsWidget)
	{
		return;
	}

	ActiveSettingsWidget->OnSettingsClosed.AddDynamic(
		this, &UBlackoutMainMenuWidget::HandleSettingsClosed);
	ActiveSettingsWidget->AddToViewport(120);
	SetVisibility(ESlateVisibility::Hidden);
}

void UBlackoutMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(),
	                               EQuitPreference::Quit, false);
}

void UBlackoutMainMenuWidget::HandleBackClicked()
{
	CloseMenu();
}

void UBlackoutMainMenuWidget::CloseMenu()
{
	OnMenuClosed.Broadcast();
	RemoveFromParent();
}

void UBlackoutMainMenuWidget::HandleSettingsClosed()
{
	if (!ActiveSettingsWidget)
	{
		return;
	}

	ActiveSettingsWidget->OnSettingsClosed.RemoveDynamic(
		this, &UBlackoutMainMenuWidget::HandleSettingsClosed);
	ActiveSettingsWidget = nullptr;
	SetVisibility(ESlateVisibility::Visible);
}

void UBlackoutMainMenuWidget::HandleLoginAttemptFinished(bool bSuccess,
	const FString& PlayerName)
{
	if (ActiveLoginWidget)
	{
		ActiveLoginWidget->OnLoginAttemptFinished.RemoveDynamic(
			this, &UBlackoutMainMenuWidget::HandleLoginAttemptFinished);
		ActiveLoginWidget = nullptr;
	}

	if (bSuccess)
	{
		RefreshForLoginState();
		
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>())
			{
				MatchmakingSubsystem->CheckActiveSession();
			}
		}
	}
}

void UBlackoutMainMenuWidget::HandleMatchmakingError(int32 HttpStatus,
	const FString& Message)
{
	if (HttpStatus != 401)
	{
		return;
	}
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->
			GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->Logout();
		}
	}
	RefreshForLoginState();
}

void UBlackoutMainMenuWidget::HandleMatchmakingWidgetExited(bool bSuccess)
{
	if (ActiveMatchmakingWidget)
	{
		ActiveMatchmakingWidget->OnMatchmakingExited.RemoveDynamic(
			this, &UBlackoutMainMenuWidget::HandleMatchmakingWidgetExited);
		ActiveMatchmakingWidget = nullptr;
	}

	if (bSuccess)
	{
		RefreshForLoginState();
	}
}

void UBlackoutMainMenuWidget::HandleActiveSessionFound()
{
	if (ReconnectButton)
	{
		ReconnectButton ->SetVisibility(ESlateVisibility::Visible);
	}
}

void UBlackoutMainMenuWidget::RefreshForLoginState()
{
	if (bUseAsInGameMenu)
	{
		if (LoginButton)
		{
			LoginButton->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (LogoutButton)
		{
			LogoutButton->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (StartMatchmakingButton)
		{
			StartMatchmakingButton->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (ReconnectButton)
		{
			ReconnectButton ->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (SinglePlayButton)
		{
			SinglePlayButton->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (WelcomeText)
		{
			WelcomeText->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (BackButton)
		{
			BackButton->SetVisibility(ESlateVisibility::Visible);
		}
		return;
	}

	const UGameInstance* GameInstance = GetGameInstance();
	const UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance
		? GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>()
		: nullptr;
	const bool bLoggedIn = MatchmakingSubsystem && MatchmakingSubsystem->
		IsLoggedIn();

	if (LoginButton)
	{
		LoginButton->SetVisibility(bLoggedIn
			                           ? ESlateVisibility::Collapsed
			                           : ESlateVisibility::Visible);
	}
	if (LogoutButton)
	{
		LogoutButton->SetVisibility(bLoggedIn
			                            ? ESlateVisibility::Visible
			                            : ESlateVisibility::Collapsed);
	}
	if (StartMatchmakingButton)
	{
		StartMatchmakingButton->SetIsEnabled(bLoggedIn);
	}
	
	if (ReconnectButton)
	{
		ReconnectButton ->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	if (WelcomeText)
	{
		WelcomeText->SetVisibility(bLoggedIn
			                           ? ESlateVisibility::Visible
			                           : ESlateVisibility::Collapsed);
		if (bLoggedIn)
		{
			WelcomeText->SetText(FText::FromString(
				FString::Printf(
					TEXT("Welcome, %s"),
					*MatchmakingSubsystem->GetPlayerName())));
		}
	}
}
