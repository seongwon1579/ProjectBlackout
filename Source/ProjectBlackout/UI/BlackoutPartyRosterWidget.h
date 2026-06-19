// ─── 구현 내역 ───────────────────────
//  - 김민영: 파티원 상태 패널 로스터 HUD 위젯 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutPartyTypes.h"
#include "BlackoutPartyRosterWidget.generated.h"

class ABlackoutPlayerState;
class UBlackoutPartyMemberStatusWidget;
class UBlackoutPartyRosterWidgetController;
class UVerticalBox;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutPartyRosterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void SetWidgetController(UBlackoutPartyRosterWidgetController* InWidgetController);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void RebuildRoster(const TArray<FBlackoutPartyMemberStatusData>& MemberStatusList);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void UpdateMemberStatus(const FBlackoutPartyMemberStatusData& MemberStatusData);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UVerticalBox> PartyMemberContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	TSubclassOf<UBlackoutPartyMemberStatusWidget> PartyMemberWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	TObjectPtr<UBlackoutPartyRosterWidgetController> WidgetController;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Roster Rebuilt"), Category = "Blackout|HUD|Party")
	void ReceiveRosterRebuilt(const TArray<FBlackoutPartyMemberStatusData>& MemberStatusList);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Member Status Updated"), Category = "Blackout|HUD|Party")
	void ReceiveMemberStatusUpdated(const FBlackoutPartyMemberStatusData& MemberStatusData);

private:
	void UnbindWidgetControllerCallbacks();
	UBlackoutPartyMemberStatusWidget* CreateMemberWidget(const FBlackoutPartyMemberStatusData& MemberStatusData);
	void RemoveMissingMemberWidgets(const TSet<TObjectKey<ABlackoutPlayerState>>& ValidPlayerStates);

	UFUNCTION()
	void HandleRosterRebuilt(const TArray<FBlackoutPartyMemberStatusData>& MemberStatusList);

	UFUNCTION()
	void HandleMemberStatusChanged(const FBlackoutPartyMemberStatusData& MemberStatusData);

	TMap<TObjectKey<ABlackoutPlayerState>, TWeakObjectPtr<UBlackoutPartyMemberStatusWidget>> MemberWidgets;
};
