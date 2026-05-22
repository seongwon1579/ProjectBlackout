// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/BlackoutClassSelectTypes.h"
#include "BlackoutClassSelectWidget.generated.h"

class UBlackoutClassSelectWidgetController;

/**
 * 캐릭터 선택 UI 위젯
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutClassSelectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void SetWidgetController(UBlackoutClassSelectWidgetController* InController);
	
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void RequestNavigateNext();
	
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void RequestNavigatePrevious();
	
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void RequestConfirm();
	
protected:
	virtual void NativeDestruct() override;
	
	UPROPERTY(Transient , BlueprintReadOnly ,Category="Blackout|ClassSelect")
	TObjectPtr<UBlackoutClassSelectWidgetController> WidgetController;
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Selection Changed"), Category="Blackout|ClassSelect")
	void ReceiveSelectChanged(const FBlackoutClassSelectDisplayData& DisplayData);
	
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Selection Confirmed"), Category="Blackout|ClassSelect")
	void ReceiveSelectConfirmed();
	
private:
	UFUNCTION()
	void HandleSelectionChanged(const FBlackoutClassSelectDisplayData& DisplayData);

	UFUNCTION()
	void HandleSelectionConfirmed();
	
	void UnbindWidgetControllerCallbacks();
};
