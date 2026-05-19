#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "GameplayTagContainer.h"
#include "UI/BlackoutConsumableTypes.h"
#include "UI/BlackoutHUDTypes.h"
#include "UI/BlackoutWeaponAmmoTypes.h"
#include "BlackoutHUDWidget.generated.h"

class ABOWeaponBase;
class UBlackoutDamageNumberWidget;
class UBlackoutConsumableSlotsWidget;
class UBlackoutHUDWidgetController;
class UBlackoutInteractionPromptWidget;
class UBlackoutPartyRosterWidget;
class UBlackoutPartyRosterWidgetController;
class UBlackoutRelicWidget;
class UBlackoutReviveProgressWidget;
class UBlackoutValueBarWidget;
class UBlackoutWeaponAmmoWidget;
class UCanvasPanel;
class UProgressBar;
class UTextBlock;
class UWidget;

UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	virtual void SetWidgetController(UBlackoutHUDWidgetController* InWidgetController);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutHUDWidgetController* GetWidgetController() const { return WidgetController; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD")
	void SetPartyRosterWidgetController(UBlackoutPartyRosterWidgetController* InPartyRosterWidgetController);

	UFUNCTION(BlueprintPure, Category = "Blackout|HUD")
	UBlackoutPartyRosterWidgetController* GetPartyRosterWidgetController() const { return PartyRosterWidgetController; }

	bool ShowDamageNumberAtWorldLocation(float DamageAmount, const FVector& WorldLocation, bool bIsCritical);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;
	virtual void NativeDestruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutHUDWidgetController> WidgetController;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD")
	TObjectPtr<UBlackoutPartyRosterWidgetController> PartyRosterWidgetController;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutValueBarWidget> HealthBarWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutValueBarWidget> StaminaBarWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutWeaponAmmoWidget> AmmoWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutConsumableSlotsWidget> ConsumableSlotsWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutRelicWidget> RelicWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutPartyRosterWidget> PartyRosterWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UWidget> ImpactIndicatorWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutInteractionPromptWidget> RevivePromptWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UBlackoutReviveProgressWidget> ReviveProgressWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UWidget> RevivePromptContainer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UTextBlock> RevivePromptTextWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UTextBlock> ReviveStatusTextWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UProgressBar> ReviveProgressBarWidget;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD")
	TObjectPtr<UCanvasPanel> CNV_DamageNumbers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	TSubclassOf<UBlackoutDamageNumberWidget> DamageNumberWidgetClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD", meta = (ClampMin = 0.0f))
	float DamageNumberRandomOffsetX = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD", meta = (ClampMin = 0.0f))
	float DamageNumberRandomOffsetY = 12.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor TrajectoryNormalColor = FLinearColor(1.f, 1.f, 1.f, 0.75f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor TrajectoryFuseInactiveColor = FLinearColor(1.0f, 0.05f, 0.02f, 0.8f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor TrajectoryOccludedColor = FLinearColor(1.0f, 0.7f, 0.0f, 0.5f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD", meta = (ClampMin = 1.0f))
	float TrajectoryDotSize = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor ImpactIndicatorDefaultColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor ImpactIndicatorMismatchColor = FLinearColor::Red;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor ImpactIndicatorOccludedColor = FLinearColor(1.0f, 0.7f, 0.0f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor RevivePromptDefaultColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FLinearColor RevivePromptErrorColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);

	/** 부활 프롬프트를 대상 월드 좌표보다 화면에서 얼마나 더 아래로 내릴지 조정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FVector2D RevivePromptScreenOffset = FVector2D(0.0f, 24.0f);

	/** 실제 부활 진행 UI를 화면 하단 중앙 근처에 고정할 때 사용하는 앵커입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FAnchors ReviveProgressScreenAnchors = FAnchors(0.5f, 0.78f, 0.5f, 0.78f);

	/** 실제 부활 진행 UI를 화면 기준으로 얼마나 더 이동할지 조정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD")
	FVector2D ReviveProgressScreenOffset = FVector2D::ZeroVector;

	/**
	 * 탄퍼짐이 최대일 때 인디케이터 위젯에 적용되는 RenderScale 배율.
	 * 1.0 = 원본 크기, 값이 클수록 최대 탄퍼짐 시 인디케이터가 더 커집니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|HUD", meta = (ClampMin = 1.0f))
	float MaxSpreadIndicatorScale = 3.0f;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Widget Controller Set"), Category = "Blackout|HUD")
	void ReceiveWidgetControllerSet();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Party Roster Controller Set"), Category = "Blackout|HUD")
	void ReceivePartyRosterControllerSet(UBlackoutPartyRosterWidgetController* InPartyRosterWidgetController);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Health Changed"), Category = "Blackout|HUD")
	void ReceiveHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Stamina Changed"), Category = "Blackout|HUD")
	void ReceiveStaminaChanged(float CurrentStamina, float MaxStamina);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Ammo Changed"), Category = "Blackout|HUD")
	void ReceiveAmmoChanged(int32 ClipAmmo, int32 MaxClipAmmo, int32 ReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Equipped Weapon Changed"), Category = "Blackout|HUD")
	void ReceiveEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Aiming Changed"), Category = "Blackout|HUD")
	void ReceiveAimingChanged(bool bIsAiming, int32 CrosshairType);

	/**
	 * 매 틱 호출됩니다. 외부 크로스헤어 에셋에 현재 탄퍼짐을 전달할 때 사용하세요.
	 * @param NormalizedSpread 0(기본 탄퍼짐) ~ 1(최대 탄퍼짐)
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Spread Updated"), Category = "Blackout|HUD")
	void ReceiveSpreadUpdated(float NormalizedSpread);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Weapon Ammo Display Changed"), Category = "Blackout|HUD")
	void ReceiveWeaponAmmoDisplayChanged(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
		bool bPlaySwapAnimation);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Relic Charges Changed"), Category = "Blackout|HUD")
	void ReceiveRelicChargesChanged(int32 CurrentCharges, int32 MaxCharges);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumables Changed"), Category = "Blackout|HUD")
	void ReceiveConsumablesChanged(int32 BloodRootCount, int32 GulSerumCount);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Consumable Slots Changed"), Category = "Blackout|HUD")
	void ReceiveConsumableSlotsChanged(
		const FBlackoutConsumableSlotData& BloodRootData,
		const FBlackoutConsumableSlotData& GulSerumData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Interaction Prompt Updated"), Category = "Blackout|HUD|Interaction")
	void ReceiveInteractionPromptUpdated(const FBlackoutInteractionPromptData& InteractionPromptData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Revive Prompt Updated", DeprecatedFunction, DeprecationMessage = "On Interaction Prompt Updated를 사용하세요."), Category = "Blackout|HUD|Revive")
	void ReceiveRevivePromptUpdated(const FBlackoutInteractionPromptData& RevivePromptData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Damage Number Requested"), Category = "Blackout|HUD")
	void ReceiveDamageNumberRequested(float DamageAmount, FVector2D ScreenPosition, bool bIsCritical);

private:
	void UnbindWidgetControllerCallbacks();
	void EnsureRevivePromptWidget();
	void EnsureReviveProgressWidget();
	void ResolveRevivePromptBindingsFromTree();
	void ResolveReviveProgressBindingsFromTree();
	void UpdateImpactIndicator(const FBlackoutImpactIndicatorData& ImpactIndicatorData);
	void UpdateInteractionPrompt(const FBlackoutInteractionPromptData& InteractionPromptData);
	void ApplyImpactIndicatorColor(const FLinearColor& IndicatorColor) const;
	int32 PaintProjectileTrajectoryDots(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle) const;
	FLinearColor ResolveTrajectoryColor(EBlackoutTrajectoryVisualState VisualState) const;

	UPROPERTY(Transient)
	TArray<FBlackoutTrajectoryPointData> CachedTrajectoryPoints;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleStaminaChanged(float CurrentStamina, float MaxStamina);

	UFUNCTION()
	void HandleAmmoChanged(int32 ClipAmmo, int32 MaxClipAmmo, int32 ReserveAmmo);

	UFUNCTION()
	void HandleEquippedWeaponChanged(ABOWeaponBase* EquippedWeapon, FGameplayTag WeaponSlotTag);

	UFUNCTION()
	void HandleAimingChanged(bool bIsAiming, int32 CrosshairType);

	UFUNCTION()
	void HandleWeaponAmmoDisplayChanged(
		const FBlackoutWeaponAmmoSlotData& PrimaryWeaponData,
		const FBlackoutWeaponAmmoSlotData& SecondaryWeaponData,
		bool bPlaySwapAnimation);

	UFUNCTION()
	void HandleRelicChargesChanged(int32 CurrentCharges, int32 MaxCharges);

	UFUNCTION()
	void HandleConsumablesChanged(int32 BloodRootCount, int32 GulSerumCount);

	UFUNCTION()
	void HandleConsumableSlotsChanged(
		const FBlackoutConsumableSlotData& BloodRootData,
		const FBlackoutConsumableSlotData& GulSerumData);
};
