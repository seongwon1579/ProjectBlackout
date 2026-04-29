#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutHUDWidget.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	virtual void SetWidgetController(UObject* InWidgetController);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UObject* GetWidgetController() const { return WidgetController; }

protected:
	virtual void NativeOnInitialized() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UObject> WidgetController;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Widget Controller Set"), Category = "Blackout|HUD")
	void ReceiveWidgetControllerSet();
};
