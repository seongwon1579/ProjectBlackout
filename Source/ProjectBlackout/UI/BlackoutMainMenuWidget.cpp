// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutMainMenuWidget.h"

#include "BlackoutLoginWidget.h"
#include "BlackoutMatchmakingWidget.h"
#include "BlackoutSettingsWidget.h"
#include "Framework/BlackoutMatchmakingSubsystem.h"

#include  "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetSystemLibrary.h"

void UBlackoutMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->
			GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->OnMatchmakingError.AddDynamic(
				this, &UBlackoutMainMenuWidget::HandleMatchmakingError);
		}
	}

	RefreshForLoginState();
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
	Super::NativeDestruct();
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

void UBlackoutMainMenuWidget::HandleOptionsClicked()
{
	if (ActiveSettingsWidget)
	{
		return;
	}

	TSubclassOf<UBlackoutSettingsWidget> ResolvedSettingsClass = SettingsWidgetClass;
	if (!ResolvedSettingsClass)
	{
		ResolvedSettingsClass = UBlackoutSettingsWidget::StaticClass();
	}

	ActiveSettingsWidget = CreateWidget<UBlackoutSettingsWidget>(
		GetOwningPlayer(), ResolvedSettingsClass);
	if (!ActiveSettingsWidget)
	{
		return;
	}

	ActiveSettingsWidget->OnSettingsClosed.AddDynamic(
		this, &UBlackoutMainMenuWidget::HandleSettingsClosed);
	ActiveSettingsWidget->AddToViewport(120);
}

void UBlackoutMainMenuWidget::HandleQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(),
	                               EQuitPreference::Quit, false);
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

void UBlackoutMainMenuWidget::RefreshForLoginState()
{
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
