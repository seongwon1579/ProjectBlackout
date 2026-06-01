#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutSpectatorWidget.generated.h"

class UTextBlock;

/**
 * 사망 후 관전 상태에서 현재 ViewTarget의 닉네임을 표시하는 위젯입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutSpectatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 컨트롤러가 매 틱 폴링해 전달하는 관전 대상 닉네임입니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Spectator")
	void SetSpectatorTargetName(const FText& InTargetName);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD|Spectator")
	const FText& GetSpectatorTargetName() const { return CurrentTargetName; }

protected:
	virtual void NativePreConstruct() override;

	void ResolveOptionalWidgetsFromTree();

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Spectator")
	FText CurrentTargetName;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Spectator")
	TObjectPtr<UTextBlock> TargetNameText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|HUD|Spectator")
	FLinearColor TargetNameTextColor = FLinearColor::White;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Spectator Target Name Changed"), Category = "Blackout|HUD|Spectator")
	void ReceiveSpectatorTargetNameChanged(const FText& InTargetName);
};
