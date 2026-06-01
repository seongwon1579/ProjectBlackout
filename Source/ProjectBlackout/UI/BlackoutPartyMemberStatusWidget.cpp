#include "UI/BlackoutPartyMemberStatusWidget.h"

#include "Characters/BlackoutPlayerCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Framework/BlackoutPlayerState.h"

void UBlackoutPartyMemberStatusWidget::SetStatusData(const FBlackoutPartyMemberStatusData& InStatusData)
{
	const bool bWasDowned = StatusData.bIsDowned;
	StatusData = InStatusData;

	if (CachedMemberCharacter.IsStale() || !CachedMemberCharacter.IsValid())
	{
		CachedMemberCharacter = ResolveMemberPlayerCharacter();
	}

	if (PlayerNameText)
	{
		PlayerNameText->SetText(StatusData.DisplayName);
	}

	RefreshStatusBar();

	const ESlateVisibility DownedVisibility = StatusData.bIsDowned
		? ESlateVisibility::HitTestInvisible
		: ESlateVisibility::Collapsed;

	if (DownedIconWidget)
	{
		DownedIconWidget->SetVisibility(DownedVisibility);
	}

	if (ReviveTextWidget)
	{
		ReviveTextWidget->SetVisibility(DownedVisibility);
		if (StatusData.bIsReviveInteractionActive)
		{
			ReviveTextWidget->SetText(RevivingStatusText);
			ReviveTextWidget->SetColorAndOpacity(RevivingStatusTextColor);
		}
		else
		{
			ReviveTextWidget->SetText(StatusData.bIsDowned ? DownedStatusText : FText::GetEmpty());
			ReviveTextWidget->SetColorAndOpacity(StatusData.bIsDowned ? DownedStatusTextColor : DefaultStatusTextColor);
		}
	}

	ReceiveStatusDataChanged(StatusData);

	if (bWasDowned != StatusData.bIsDowned)
	{
		ReceiveDownedStateChanged(StatusData.bIsDowned);
	}
}

void UBlackoutPartyMemberStatusWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!StatusData.bIsValid || !StatusData.bIsDowned || StatusData.bIsDead)
	{
		return;
	}

	if (!CachedMemberCharacter.IsValid())
	{
		CachedMemberCharacter = ResolveMemberPlayerCharacter();
	}

	const ABlackoutPlayerCharacter* MemberCharacter = CachedMemberCharacter.Get();
	if (!MemberCharacter)
	{
		return;
	}

	// 서버 권위 타이머가 매 프레임 흐르므로, 다운 중일 동안에는 위젯이 직접 폴링해서 잔여 시간을 최신화합니다.
	if (StatusData.bIsReviveInteractionActive)
	{
		const float ReviveTotal = FMath::Max(KINDA_SMALL_NUMBER, MemberCharacter->GetReviveDuration());
		const float ReviveRemaining = FMath::Clamp(MemberCharacter->GetReviveRemainingTime(), 0.0f, ReviveTotal);
		StatusData.HUDMode = EBlackoutHUDMode::DownedReviveTimer;
		StatusData.TimerTotalDuration = ReviveTotal;
		StatusData.TimerRemainingTime = ReviveRemaining;
		StatusData.TimerProgressNormalized = FMath::Clamp(1.0f - (ReviveRemaining / ReviveTotal), 0.0f, 1.0f);
	}
	else
	{
		const float DeathTotal = FMath::Max(KINDA_SMALL_NUMBER, MemberCharacter->GetDownedDeathDuration());
		const float DeathRemaining = FMath::Clamp(MemberCharacter->GetDownedDeathRemainingTime(), 0.0f, DeathTotal);
		StatusData.HUDMode = EBlackoutHUDMode::DownedDeathTimer;
		StatusData.TimerTotalDuration = DeathTotal;
		StatusData.TimerRemainingTime = DeathRemaining;
		StatusData.TimerProgressNormalized = FMath::Clamp(1.0f - (DeathRemaining / DeathTotal), 0.0f, 1.0f);
	}

	RefreshStatusBar();
}

void UBlackoutPartyMemberStatusWidget::RefreshStatusBar()
{
	if (!StatusBar)
	{
		return;
	}

	float Percent = 0.0f;
	FLinearColor BarColor = HealthBarColor;

	if (StatusData.bIsDowned && !StatusData.bIsDead && StatusData.TimerTotalDuration > KINDA_SMALL_NUMBER)
	{
		if (StatusData.bIsReviveInteractionActive)
		{
			// 부활 타이머: 진행될수록 채워짐.
			Percent = FMath::Clamp(StatusData.TimerProgressNormalized, 0.0f, 1.0f);
			BarColor = ReviveTimerBarColor;
		}
		else
		{
			// 사망 타이머: 남은 시간이 줄어들수록 비워짐.
			Percent = FMath::Clamp(StatusData.TimerRemainingTime / StatusData.TimerTotalDuration, 0.0f, 1.0f);
			BarColor = DeathTimerBarColor;
		}
	}
	else
	{
		Percent = StatusData.MaxHealth > 0.0f
			? FMath::Clamp(StatusData.CurrentHealth / StatusData.MaxHealth, 0.0f, 1.0f)
			: 0.0f;
		BarColor = HealthBarColor;
	}

	StatusBar->SetPercent(Percent);
	StatusBar->SetFillColorAndOpacity(BarColor);
}

ABlackoutPlayerCharacter* UBlackoutPartyMemberStatusWidget::ResolveMemberPlayerCharacter() const
{
	const ABlackoutPlayerState* MemberPlayerState = StatusData.PlayerState.Get();
	if (!MemberPlayerState)
	{
		return nullptr;
	}

	return Cast<ABlackoutPlayerCharacter>(MemberPlayerState->GetPawn());
}
