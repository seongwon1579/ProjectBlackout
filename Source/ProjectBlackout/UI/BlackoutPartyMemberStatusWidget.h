// ─── 구현 내역 ───────────────────────
//  - 김민영: 파티원 개별 상태 위젯 및 다운/부활 타이머 표시 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutPartyTypes.h"
#include "BlackoutPartyMemberStatusWidget.generated.h"

class ABlackoutPlayerCharacter;
class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutPartyMemberStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void SetStatusData(const FBlackoutPartyMemberStatusData& InStatusData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Party")
	const FBlackoutPartyMemberStatusData& GetStatusData() const { return StatusData; }

protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FBlackoutPartyMemberStatusData StatusData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UTextBlock> PlayerNameText;

	/** 체력/사망 타이머/부활 타이머를 상태에 따라 한 바에서 표시합니다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UProgressBar> StatusBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UWidget> DownedIconWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Party")
	TObjectPtr<UTextBlock> ReviveTextWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FText DownedStatusText = FText::FromString(TEXT("쓰러짐"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FText RevivingStatusText = FText::FromString(TEXT("부활 중"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FSlateColor DefaultStatusTextColor = FSlateColor(FLinearColor::White);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FSlateColor DownedStatusTextColor = FSlateColor(FLinearColor(1.0f, 0.15f, 0.1f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FSlateColor RevivingStatusTextColor = FSlateColor(FLinearColor(0.2f, 0.75f, 1.0f, 1.0f));

	/** 정상 체력 상태일 때 StatusBar 색상입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Party|Bar")
	FLinearColor HealthBarColor = FLinearColor(0.15f, 0.85f, 0.25f, 1.0f);

	/** 다운 사망 타이머가 흐르는 동안 StatusBar 색상입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Party|Bar")
	FLinearColor DeathTimerBarColor = FLinearColor(0.85f, 0.15f, 0.15f, 1.0f);

	/** 부활 진행 중일 때 StatusBar 색상입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Party|Bar")
	FLinearColor ReviveTimerBarColor = FLinearColor(0.2f, 0.75f, 1.0f, 1.0f);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Status Data Changed"), Category = "Blackout|HUD|Party")
	void ReceiveStatusDataChanged(const FBlackoutPartyMemberStatusData& InStatusData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Downed State Changed"), Category = "Blackout|HUD|Party")
	void ReceiveDownedStateChanged(bool bIsDowned);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/** StatusBar의 채움 비율·색상을 체력/사망/부활 상태에 맞춰 갱신합니다. */
	void RefreshStatusBar();

	/** StatusData.PlayerState로부터 폰을 캐스팅해 캐시합니다. */
	ABlackoutPlayerCharacter* ResolveMemberPlayerCharacter() const;

	mutable TWeakObjectPtr<ABlackoutPlayerCharacter> CachedMemberCharacter;
};
