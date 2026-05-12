// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutMatchmakingWidget.h"

#include "Framework/BlackoutMatchmakingSubsystem.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UBlackoutMatchmakingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandleCancelClicked);
	}
	UGameInstance* GameInstance = GetGameInstance();
	UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance
		? GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>()
		: nullptr;
	if (MatchmakingSubsystem)
	{
		MatchmakingSubsystem->OnMatchmakingStarted.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandleMatchmakingStarted);
		MatchmakingSubsystem->OnPlayerJoined.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandlePlayerJoined);
		MatchmakingSubsystem->OnPlayerLeft.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandlePlayerLeft);
		MatchmakingSubsystem->OnMatchmakingFailed.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandleMatchmakingFailed);
		MatchmakingSubsystem->OnSessionCancelled.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandleSessionCancelled);
		MatchmakingSubsystem->OnMatchmakingError.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandleMatchmakingError);
		MatchmakingSubsystem->OnMatchmakingCancelled.AddDynamic(
			this, &UBlackoutMatchmakingWidget::HandleMatchmakingCancelled);
	}

	if (StatusText)
	{
		StatusText->SetText(
			NSLOCTEXT("BlackoutMatchmaking", "Waiting", "매칭 중…"));
	}
	if (PlayerCountText)
	{
		PlayerCountText->SetText(FText::GetEmpty());
	}

	if (MatchmakingSubsystem)
	{
		MatchmakingSubsystem->StartMatchmaking();
	}
}

void UBlackoutMatchmakingWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance->
			GetSubsystem<UBlackoutMatchmakingSubsystem>())
		{
			MatchmakingSubsystem->OnMatchmakingStarted.RemoveDynamic(
				this, &UBlackoutMatchmakingWidget::HandleMatchmakingStarted);
			MatchmakingSubsystem->OnPlayerJoined.RemoveDynamic(
				this, &UBlackoutMatchmakingWidget::HandlePlayerJoined);
			MatchmakingSubsystem->OnPlayerLeft.RemoveDynamic(
				this, &UBlackoutMatchmakingWidget::HandlePlayerLeft);
			MatchmakingSubsystem->OnMatchmakingFailed.RemoveDynamic(
				this, &UBlackoutMatchmakingWidget::HandleMatchmakingFailed);
			MatchmakingSubsystem->OnSessionCancelled.RemoveDynamic(
				this, &UBlackoutMatchmakingWidget::HandleSessionCancelled);
			MatchmakingSubsystem->OnMatchmakingError.RemoveDynamic(
				this, &UBlackoutMatchmakingWidget::HandleMatchmakingError);
			MatchmakingSubsystem->OnMatchmakingCancelled.RemoveDynamic(
				this, &UBlackoutMatchmakingWidget::HandleMatchmakingCancelled);
		}
	}

	Super::NativeDestruct();
}

void UBlackoutMatchmakingWidget::HandleCancelClicked()
{
	if (CancelButton)
	{
		CancelButton->SetIsEnabled(false);
	}
	if (StatusText)
	{
		StatusText->SetText(NSLOCTEXT("BlackoutMatchmaking", "Cancelling",
		                              "취소 중…"));
	}
	UGameInstance* GameInstance = GetGameInstance();
	UBlackoutMatchmakingSubsystem* MatchmakingSubsystem = GameInstance
		? GameInstance->GetSubsystem<UBlackoutMatchmakingSubsystem>()
		: nullptr;
	if (MatchmakingSubsystem)
	{
		MatchmakingSubsystem->CancelMatchmaking();
	}
	else
	{
		ExitWithResult(false);
	}
}

void UBlackoutMatchmakingWidget::HandleMatchmakingStarted(
	const FBlackoutSessionInfo& SessionInfo)
{
	CachedMaxPlayers = SessionInfo.MaxPlayers;
	RefreshPlayerCount(SessionInfo.Players.Num());
}

void UBlackoutMatchmakingWidget::HandlePlayerJoined(const FString& SessionId,
                                                    const TArray<FString>&
                                                    Players, int32 Count)
{
	RefreshPlayerCount(Count);
}

void UBlackoutMatchmakingWidget::HandlePlayerLeft(const FString& SessionId,
                                                  const FString& PlayerName,
                                                  const TArray<FString>&
                                                  Players, int32 Count)
{
	RefreshPlayerCount(Count);
}

void UBlackoutMatchmakingWidget::HandleMatchmakingFailed(
	const FString& SessionId, const FString& Reason)
{
	if (StatusText)
	{
		StatusText->SetText(NSLOCTEXT("BlackoutMatchmaking", "Timeout",
		                              "매칭 시간 초과"));
	}
	ExitWithResult(false);
}

void UBlackoutMatchmakingWidget::HandleSessionCancelled(
	const FString& SessionId)
{
	if (StatusText)
	{
		StatusText->SetText(NSLOCTEXT("BlackoutMatchmaking", "SessionCancelled",
		                              "세션이 종료되었습니다"));
	}
	ExitWithResult(false);
}

void UBlackoutMatchmakingWidget::HandleMatchmakingError(int32 HttpStatus,
	const FString& Message)
{
	if (StatusText)
	{
		if (HttpStatus == 409)
		{
			StatusText->SetText(NSLOCTEXT("BlackoutMatchmaking",
										  "DuplicateName",
										  "같은 닉네임이 이미 매칭 중입니다.\n다른 닉네임으로 다시 로그인하세요"));
		}
		else
		{
			StatusText->SetText(NSLOCTEXT("BlackoutMatchmaking", "Error", "매칭 에러가 발생했습니다"));
		}
	}
	ExitWithResult(false);
}

void UBlackoutMatchmakingWidget::HandleMatchmakingCancelled(
	bool bSessionDestroyed)
{
	if (StatusText)
	{
		StatusText->SetText(NSLOCTEXT("BlackoutMatchmaking","Cancelled","취소되었습니다"));
	}
	ExitWithResult(false);
}

void UBlackoutMatchmakingWidget::RefreshPlayerCount(int32 Count)
{
	if (!PlayerCountText)
	{
		return;
	}
	const int32 Max = CachedMaxPlayers > 0 ? CachedMaxPlayers : 4;
	PlayerCountText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"),Count,Max)));
}

void UBlackoutMatchmakingWidget::ExitWithResult(bool bSuccess)
{
	OnMatchmakingExited.Broadcast(bSuccess);
	RemoveFromParent();
}
