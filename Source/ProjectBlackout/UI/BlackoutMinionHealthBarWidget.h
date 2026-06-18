// ─── 구현 내역 ───────────────────────
//  - 김민영: 미니언 체력바 위젯 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutMinionHealthBarWidget.generated.h"

class UAbilitySystemComponent;
class UProgressBar;
struct FOnAttributeChangeData;

/**
 * 미니언 머리 위에 표시되는 월드 스페이스 체력바 위젯.
 * 체력이 최대치일 때와 고갈됐을 때는 표시하지 않고, 피해를 받은 동안만 표시합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutMinionHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|MinionHealthBar")
	void BindToAbilitySystem(UAbilitySystemComponent* InAbilitySystemComponent);

	UFUNCTION(BlueprintCallable, Category = "Blackout|MinionHealthBar")
	void UnbindFromAbilitySystem();

	UFUNCTION(BlueprintCallable, Category = "Blackout|MinionHealthBar")
	void RefreshHealth();

	UFUNCTION(BlueprintCallable, Category = "Blackout|MinionHealthBar")
	void SetForceHidden(bool bInForceHidden);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|MinionHealthBar")
	float CurrentHealth = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|MinionHealthBar")
	float MaxHealth = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|MinionHealthBar")
	float NormalizedHealth = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|MinionHealthBar")
	bool bShouldShowHealthBar = false;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Health Changed"), Category = "Blackout|MinionHealthBar")
	void ReceiveHealthChanged(float NewCurrentHealth, float NewMaxHealth, float NewNormalizedHealth);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Visibility Rule Changed"), Category = "Blackout|MinionHealthBar")
	void ReceiveVisibilityRuleChanged(bool bNewShouldShowHealthBar);

private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void ApplyHealth(float NewCurrentHealth, float NewMaxHealth);
	void UpdateVisibilityRule();

	TWeakObjectPtr<UAbilitySystemComponent> BoundAbilitySystemComponent;
	bool bForceHidden = true;
};
