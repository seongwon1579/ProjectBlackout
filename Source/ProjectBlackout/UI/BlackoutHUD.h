// ─── 구현 내역 ───────────────────────
//  - 김민영: 인게임 HUD 기본 클래스 및 위젯 컨트롤러 바인딩 구현
//  - 조성원: 보스 체력 동기화 적 HUD 연동
//  - 허혁: 데미지 숫자 위젯 연동
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlackoutHUD.generated.h"

class UBlackoutHUDWidget;
class UBlackoutEnemyHUDWidget;
class UBlackoutHUDWidgetController;
class UBlackoutEnemyHUDWidgetController;
class UBlackoutMatchResultWidgetController;
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

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutMatchResultWidgetController* GetMatchResultWidgetController() const { return MatchResultWidgetController; }

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

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutMatchResultWidgetController> MatchResultWidgetController;

private:
	void CreateHUDWidget();
	void CreateWidgetController();
};
