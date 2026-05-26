#include "UI/BlackoutSurrenderVoteWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "UI/BlackoutHUDWidgetController.h"
#include "Framework/BlackoutGameState.h"
#include "Framework/BlackoutPlayerState.h"

void UBlackoutSurrenderVoteWidget::SetWidgetController(UBlackoutHUDWidgetController* InWidgetController)
{
	if (!InWidgetController)
	{
		return;
	}

	// 델리게이트 구독 연동
	InWidgetController->OnSurrenderVoteStateChanged.AddUniqueDynamic(this, &UBlackoutSurrenderVoteWidget::HandleSurrenderVoteStateChanged);

	// 초기 가시성 설정 (투표 비활성화 디폴트)
	SetVisibility(ESlateVisibility::Collapsed);
}

void UBlackoutSurrenderVoteWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bIsVoteActive)
	{
		UpdateTimerDisplay();
	}
}

void UBlackoutSurrenderVoteWidget::HandleSurrenderVoteStateChanged(bool bIsActive, int32 YesCount, int32 NoCount, float EndTimeSeconds)
{
	bIsVoteActive = bIsActive;
	VoteEndTimeSeconds = EndTimeSeconds;

	// 가시성 설정
	SetVisibility(bIsActive ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	TArray<FBlackoutPlayerVoteStatus> VoteStatuses;

	if (bIsActive)
	{
		// GameState의 플레이어 목록을 순회하며 실시간 찬성/반대 의사 상태 데이터를 수집합니다.
		if (UWorld* World = GetWorld())
		{
			if (const ABlackoutGameState* GS = World->GetGameState<ABlackoutGameState>())
			{
				for (APlayerState* PSBase : GS->PlayerArray)
				{
					if (const ABlackoutPlayerState* PS = Cast<ABlackoutPlayerState>(PSBase))
					{
						FBlackoutPlayerVoteStatus Status;
						Status.PlayerName = PS->GetPlayerName();
						
						if (PS->bRequestedSurrender)
						{
							Status.VoteChoice = EBlackoutVoteChoice::Yes;
						}
						else if (PS->bVotedAgainstSurrender)
						{
							Status.VoteChoice = EBlackoutVoteChoice::No;
						}
						else
						{
							Status.VoteChoice = EBlackoutVoteChoice::Unvoted;
						}

						VoteStatuses.Add(Status);
					}
				}
			}
		}

		UpdateTimerDisplay();
	}

	// 블루프린트로 가공된 실시간 플레이어 투표 데이터 구조체 배열 전송 ➡️ LoL/발로란트 스타일 이미지 렌더링에 매핑
	ReceiveSurrenderVoteStateChanged(bIsActive, VoteStatuses, EndTimeSeconds);
}

void UBlackoutSurrenderVoteWidget::UpdateTimerDisplay()
{
	if (!GetWorld())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	const float Remaining = FMath::Max(0.f, VoteEndTimeSeconds - CurrentTime);
	const float Percent = FMath::Clamp(Remaining / 30.f, 0.f, 1.f);

	// 프로그레스 바 적용
	if (TimerProgressBar)
	{
		TimerProgressBar->SetPercent(Percent);
	}

	// 남은 시간 텍스트 적용 (예: 25.4s)
	if (TimerText)
	{
		const FString TimerStr = FString::Printf(TEXT("%.1fs"), Remaining);
		TimerText->SetText(FText::FromString(TimerStr));
	}
}
