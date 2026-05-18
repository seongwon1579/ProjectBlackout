#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlackoutHUD.generated.h"

class UBlackoutHUDWidget;
class UBlackoutEnemyHUDWidget;
class UBlackoutHUDWidgetController;
class UBlackoutEnemyHUDWidgetController;
class UBlackoutPartyRosterWidgetController;

UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABlackoutHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	void InitHUD();

	bool ShowDamageNumberAtWorldLocation(float DamageAmount, const FVector& WorldLocation, bool bIsCritical);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutHUDWidget* GetHUDWidget() const { return HUDWidget; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutHUDWidgetController* GetHUDWidgetController() const { return HUDWidgetController; }
	
	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutEnemyHUDWidgetController* GetEnemyHUDWidgetController() const { return EnemyHUDWidgetController; }

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutPartyRosterWidgetController* GetPartyRosterWidgetController() const { return PartyRosterWidgetController; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	TSubclassOf<UBlackoutEnemyHUDWidget> EnemyHUDWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutEnemyHUDWidget> EnemyHUDWidget;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	TSubclassOf<UBlackoutHUDWidget> HUDWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidget> HUDWidget;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidgetController> HUDWidgetController;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutEnemyHUDWidgetController> EnemyHUDWidgetController;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutPartyRosterWidgetController> PartyRosterWidgetController;

private:
	void CreateHUDWidget();
	void CreateWidgetController();
};
