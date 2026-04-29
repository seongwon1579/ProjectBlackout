#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlackoutHUD.generated.h"

class UBlackoutHUDWidget;

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

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	TSubclassOf<UBlackoutHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidget> HUDWidget;

private:
	void CreateHUDWidget();
};
