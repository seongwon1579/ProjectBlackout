#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlackoutHUD.generated.h"

class UBlackoutHUDWidget;
class UBlackoutHUDWidgetController;

UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABlackoutHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	void InitHUD();

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutHUDWidget* GetHUDWidget() const { return HUDWidget; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutHUDWidgetController* GetHUDWidgetController() const { return HUDWidgetController; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	TSubclassOf<UBlackoutHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidget> HUDWidget;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidgetController> HUDWidgetController;

private:
	void CreateHUDWidget();
	void CreateWidgetController();
};
