// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "UI/BlackoutClassSelectTypes.h"
#include "BlackoutClassSelectWidgetController.generated.h"

class APlayerController;
class UBOCharacterRoster;
class ABOFirearm;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
FBlackoutClassSelectionChangedSignature,
const FBlackoutClassSelectDisplayData&, DisplayData
);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBlackoutClassSelectionConfirmedSignature);


/**
 * 캐릭터 선택 UI 컨트롤러
 *  A/D → 인덱스 ±1 + Display broadcast (로컬). F → Server_SelectClass + UI 닫기 broadcast
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBlackoutClassSelectWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	bool Initialize(APlayerController* InPlayerController , UBOCharacterRoster* InRoster);
	
	/** 현재 인덱스의 Display 를 다시 broadcast. Widget 초기 표시 시 호출 */
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void BroadcastCurrentSelection();
	
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void NavigateNext(); 
	
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void NavigatePrevious();
	
	/** F 확정 , PC->Server_SelectClass 호출 , OnSelectionConfirmed broadcast */
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void ConfirmSelection();
	
	UPROPERTY(BlueprintAssignable, Category="Blackout|ClassSelect")
	FBlackoutClassSelectionChangedSignature OnSelectionChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Blackout|ClassSelect")
	FBlackoutClassSelectionConfirmedSignature OnSelectionConfirmed;
	
private:
	FBlackoutClassSelectDisplayData BuildDisplayData(int32 Index) const;
	FBlackoutFirearmStat LookupFirearmStat(TSubclassOf<ABOFirearm> WeaponClass) const;
	
	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<UBOCharacterRoster> Roster;
	int32 CurrentIndex = 0;
};
