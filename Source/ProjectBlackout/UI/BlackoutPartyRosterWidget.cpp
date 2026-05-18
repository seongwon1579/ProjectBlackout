#include "UI/BlackoutPartyRosterWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/VerticalBox.h"
#include "Core/BlackoutLog.h"
#include "Framework/BlackoutPlayerState.h"
#include "UI/BlackoutPartyMemberStatusWidget.h"
#include "UI/BlackoutPartyRosterWidgetController.h"

void UBlackoutPartyRosterWidget::SetWidgetController(UBlackoutPartyRosterWidgetController* InWidgetController)
{
	if (!InWidgetController)
	{
		BO_LOG_CORE(Warning, "파티 로스터 컨트롤러 연결 실패: WidgetController가 유효하지 않습니다.");
		return;
	}

	UnbindWidgetControllerCallbacks();
	WidgetController = InWidgetController;

	WidgetController->OnRosterRebuilt.AddDynamic(this, &UBlackoutPartyRosterWidget::HandleRosterRebuilt);
	WidgetController->OnMemberStatusChanged.AddDynamic(this, &UBlackoutPartyRosterWidget::HandleMemberStatusChanged);
}

void UBlackoutPartyRosterWidget::RebuildRoster(const TArray<FBlackoutPartyMemberStatusData>& MemberStatusList)
{
	TSet<TObjectKey<ABlackoutPlayerState>> ValidPlayerStates;

	for (const FBlackoutPartyMemberStatusData& MemberStatusData : MemberStatusList)
	{
		ABlackoutPlayerState* MemberPlayerState = MemberStatusData.PlayerState.Get();
		if (!MemberPlayerState || !MemberStatusData.bIsValid)
		{
			continue;
		}

		ValidPlayerStates.Add(MemberPlayerState);
		UpdateMemberStatus(MemberStatusData);
	}

	RemoveMissingMemberWidgets(ValidPlayerStates);
	ReceiveRosterRebuilt(MemberStatusList);
}

void UBlackoutPartyRosterWidget::UpdateMemberStatus(const FBlackoutPartyMemberStatusData& MemberStatusData)
{
	ABlackoutPlayerState* MemberPlayerState = MemberStatusData.PlayerState.Get();
	if (!MemberPlayerState || !MemberStatusData.bIsValid)
	{
		return;
	}

	const TObjectKey<ABlackoutPlayerState> PlayerStateKey(MemberPlayerState);
	UBlackoutPartyMemberStatusWidget* MemberWidget = nullptr;

	if (TWeakObjectPtr<UBlackoutPartyMemberStatusWidget>* ExistingWidget = MemberWidgets.Find(PlayerStateKey))
	{
		MemberWidget = ExistingWidget->Get();
	}

	if (!MemberWidget)
	{
		MemberWidget = CreateMemberWidget(MemberStatusData);
		if (!MemberWidget)
		{
			return;
		}

		MemberWidgets.Add(PlayerStateKey, MemberWidget);
	}

	MemberWidget->SetStatusData(MemberStatusData);
	ReceiveMemberStatusUpdated(MemberStatusData);
}

void UBlackoutPartyRosterWidget::NativeDestruct()
{
	UnbindWidgetControllerCallbacks();
	MemberWidgets.Reset();

	Super::NativeDestruct();
}

void UBlackoutPartyRosterWidget::UnbindWidgetControllerCallbacks()
{
	if (!WidgetController)
	{
		return;
	}

	WidgetController->OnRosterRebuilt.RemoveAll(this);
	WidgetController->OnMemberStatusChanged.RemoveAll(this);
}

UBlackoutPartyMemberStatusWidget* UBlackoutPartyRosterWidget::CreateMemberWidget(
	const FBlackoutPartyMemberStatusData& MemberStatusData)
{
	if (!PartyMemberWidgetClass)
	{
		BO_LOG_CORE(Warning, "파티 멤버 위젯 생성 실패: PartyMemberWidgetClass가 지정되지 않았습니다.");
		return nullptr;
	}

	if (!PartyMemberContainer)
	{
		BO_LOG_CORE(Warning, "파티 멤버 위젯 생성 실패: PartyMemberContainer가 바인딩되지 않았습니다.");
		return nullptr;
	}

	UBlackoutPartyMemberStatusWidget* MemberWidget =
		CreateWidget<UBlackoutPartyMemberStatusWidget>(GetOwningPlayer(), PartyMemberWidgetClass);
	if (!MemberWidget)
	{
		BO_LOG_CORE(Error, "파티 멤버 위젯 생성 실패: CreateWidget이 nullptr을 반환했습니다.");
		return nullptr;
	}

	PartyMemberContainer->AddChildToVerticalBox(MemberWidget);
	MemberWidget->SetStatusData(MemberStatusData);
	return MemberWidget;
}

void UBlackoutPartyRosterWidget::RemoveMissingMemberWidgets(
	const TSet<TObjectKey<ABlackoutPlayerState>>& ValidPlayerStates)
{
	for (auto It = MemberWidgets.CreateIterator(); It; ++It)
	{
		if (ValidPlayerStates.Contains(It.Key()))
		{
			continue;
		}

		if (UBlackoutPartyMemberStatusWidget* MemberWidget = It.Value().Get())
		{
			MemberWidget->RemoveFromParent();
		}

		It.RemoveCurrent();
	}
}

void UBlackoutPartyRosterWidget::HandleRosterRebuilt(
	const TArray<FBlackoutPartyMemberStatusData>& MemberStatusList)
{
	RebuildRoster(MemberStatusList);
}

void UBlackoutPartyRosterWidget::HandleMemberStatusChanged(
	const FBlackoutPartyMemberStatusData& MemberStatusData)
{
	UpdateMemberStatus(MemberStatusData);
}
