// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutLoginWidget.h"

#include "Framework/BlackoutMatchmakingSubsystem.h"
#include "Framework/BlackoutNetworkSettings.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UBlackoutLoginWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (LoginButton)
	{
		LoginButton->OnClicked.AddDynamic(this, &UBlackoutLoginWidget::HandleLoginClicked);
	}
	
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UBlackoutLoginWidget::HandleCancelClicked);
	}
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->OnLoginComplete.AddDynamic(this, &UBlackoutLoginWidget::HandleLoginComplete);
		}
	}
	
	if (ErrorMessageText)
	{
		ErrorMessageText->SetText(FText::GetEmpty());
	}
}

void UBlackoutLoginWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->OnLoginComplete.RemoveDynamic(this, &UBlackoutLoginWidget::HandleLoginComplete);
		}
	}
	Super::NativeDestruct();
}

void UBlackoutLoginWidget::HandleLoginClicked()
{
	if (!PlayerNameInput)
	{
		return;
	}
	
	const FString PlayerName = PlayerNameInput->GetText().ToString().TrimStartAndEnd();
	
	if (PlayerName.IsEmpty())
	{
		if (ErrorMessageText)
		{
			ErrorMessageText->SetText(NSLOCTEXT("BlackoutLogin", "EmptyField","이름을 입력하세요"));
		}
		return;
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance ? GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>() : nullptr;
	if (!MatchmakingSubsystem)
	{
		return;
	}
	const UBlackoutNetworkSettings* Settings = GetDefault<UBlackoutNetworkSettings>();
	const FString Password = Settings ? Settings->DummyPassword : FString();
	
	SetBusy(true);
	MatchmakingSubsystem->Login(PlayerName, Password);
}

void UBlackoutLoginWidget::HandleCancelClicked()
{
	OnLoginAttemptFinished.Broadcast(false, FString());
	RemoveFromParent();
}

void UBlackoutLoginWidget::HandleLoginComplete(bool bSuccess)
{
	if (bSuccess)
	{
		FString PlayerName;
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>())
			{
				PlayerName = MatchmakingSubsystem->GetPlayerName();
			}
		}
		OnLoginAttemptFinished.Broadcast(bSuccess, PlayerName);
		RemoveFromParent();
		return;
	}
	
	if (ErrorMessageText)
	{
		ErrorMessageText->SetText(NSLOCTEXT("BlackoutLogin","LoginFailed","로그인 실패 다시 시도하세요"));
	}
	SetBusy(false);
}

void UBlackoutLoginWidget::SetBusy(bool bBusy)
{
	if (LoginButton)
	{
		LoginButton->SetIsEnabled(!bBusy);
	}
	if (CancelButton)
	{
		CancelButton->SetIsEnabled(!bBusy);
	}
	if (bBusy && ErrorMessageText)
	{
		ErrorMessageText->SetText(FText::GetEmpty());
	}
}
