#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutDownedStateWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 로컬 플레이어가 다운 상태일 때 사망/부활 타이머 프로그래스 바를 표시하는 위젯입니다.
 * 컨트롤러가 빌드한 FBlackoutDownedStateHUDData를 화면 갱신용으로만 소비합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutDownedStateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Downed")
	void SetDownedStateHUDData(const FBlackoutDownedStateHUDData& InHUDData);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Downed")
	const FBlackoutDownedStateHUDData& GetDownedStateHUDData() const { return HUDData; }

protected:
	virtual void NativePreConstruct() override;

	/** 디자이너에서 수동 배치한 위젯을 이름 기준으로 재바인딩합니다. */
	void ResolveOptionalWidgetsFromTree();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	FBlackoutDownedStateHUDData HUDData;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Downed")
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Downed")
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	FLinearColor DefaultTextColor = FLinearColor::White;

	/** 사망 타이머가 흐르는 동안 StatusText에 표시할 문구입니다. 비어 있으면 텍스트를 숨깁니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Downed|Text")
	FText DeathTimerStatusText;

	/** 부활 시도로 사망 타이머가 일시정지된 동안 표시할 문구입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Downed|Text")
	FText ReviveTimerStatusText;

	/** 완전 사망 상태에서 사용할 예비 문구입니다. 관전 HUD 상세는 후속 작업에서 정의됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Downed|Text")
	FText SpectatorStatusText;

	/** 현재 HUDData.HUDMode에 해당하는 표시 문구를 반환합니다. 컨트롤러가 채운 StatusText가 비어 있을 때만 사용합니다. */
	FText ResolveStatusTextForMode(EBlackoutHUDMode HUDMode) const;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Downed State HUD Data Changed"), Category = "Blackout|HUD|Downed")
	void ReceiveDownedStateHUDDataChanged(const FBlackoutDownedStateHUDData& InHUDData);
};
