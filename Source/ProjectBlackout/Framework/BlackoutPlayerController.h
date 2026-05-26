#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UBlackoutAbilitySystemComponent;
class UBlackoutCombatComponent;
class AActor;
class UBlackoutClassSelectWidget;
class UBlackoutClassSelectWidgetController;

UCLASS()
class PROJECTBLACKOUT_API ABlackoutPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void AcknowledgePossession(APawn* P) override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Blackout|Controller")
	void Server_SelectClass(FGameplayTag ClassTag);
	
	UFUNCTION(Server, Reliable , BlueprintCallable , Category= "Blackout|Controller")
	void Server_SetReady(bool bNewReady);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Blackout|Controller")
	void Client_OpenClassSelectUI();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Open Class Select UI"), Category = "Blackout|Controller")
	void ReceiveOpenClassSelectUI();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Blackout|Controller")
	void Client_ShowDamageNumber(float DamageAmount, bool bIsCritical);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Show Damage Number"), Category = "Blackout|Controller")
	void ReceiveShowDamageNumber(float DamageAmount, bool bIsCritical);

	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "Blackout|Controller")
	void Client_ShowDamageNumberAtLocation(float DamageAmount, FVector WorldLocation, bool bIsCritical);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Controller")
	void EnterSpectatorMode();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Controller")
	void ExitSpectatorMode();

	UFUNCTION(Client, Reliable, Category = "Blackout|Controller")
	void Client_SetSpectateTarget(AActor* TargetActor, float BlendTime);

	UFUNCTION(Client, Reliable, Category = "Blackout|Controller")
	void Client_ReturnToOwnPawnView(float BlendTime);

	/**
	 * 사망한 관전 플레이어가 A/D 입력으로 관전 대상을 순환할 때 서버에 요청합니다.
	 * 서버가 살아있는 아군 중 현재 대상의 이전/다음을 찾아 ViewTarget을 변경합니다.
	 * @param Direction -1=이전, +1=다음.
	 */
	UFUNCTION(Server, Reliable, Category = "Blackout|Controller|Spectator")
	void Server_CycleSpectateTarget(int32 Direction);
	
	/** ESC 또는 OnSelectionConfirmed 후 자동 호출. Widget 정리 + IMC 원복. */
	UFUNCTION(BlueprintCallable,Category="Blackout|ClassSelect")
	void CloseClassSelectUI();

#pragma region InputSetup
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	/** 사망 후 관전 상태일 때만 활성화되는 입력 매핑 컨텍스트입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Spectator")
	TObjectPtr<UInputMappingContext> SpectatorMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Spectator")
	TObjectPtr<UInputAction> SpectatePrevAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Spectator")
	TObjectPtr<UInputAction> SpectateNextAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> SwapWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> DebugSelfDamageAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> UseConsumable1Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> UseConsumable2Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> UseRelicAction;
	
	/** 캐릭터 선택 UI 활성 시에만 사용하는 입력 컨텍스트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|ClassSelect")
	TObjectPtr<UInputMappingContext> ClassSelectMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectNextAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectPrevAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectConfirmAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectCancelAction;
	
	/** UMG 위젯 클래스 WBP_ClassSelect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Blackout|ClassSelect")
	TSubclassOf<UBlackoutClassSelectWidget> ClassSelectWidgetClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutClassSelectWidget> ClassSelectWidget;
	
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutClassSelectWidgetController> ClassSelectController;
	
	void OnClassSelectNextPressed();
	void OnClassSelectPrevPressed();
	void OnClassSelectConfirmPressed();
	void OnClassSelectCancelPressed();

	UFUNCTION()
	void HandleClassSelectionConfirmed();

	void OnFirePressed();
	void OnFireReleased();
	void OnAimPressed();
	void OnAimReleased();
	void OnReloadPressed();
	void OnSwapWeaponPressed();
	void OnDodgePressed();
	void OnSprintPressed();
	void OnSprintReleased();
	void OnDebugSelfDamagePressed();
	void OnInteractPressed();
	void OnInteractReleased();
	void OnUseConsumable1Pressed();
	void OnUseConsumable1Released();
	void OnUseConsumable2Pressed();
	void OnUseConsumable2Released();
	void OnUseRelicPressed();
	void OnUseRelicReleased();
	void OnSpectatePrevPressed();
	void OnSpectateNextPressed();

	/** 관전 진입/이탈 시 SpectatorMappingContext를 푸시/팝합니다. */
	void SetSpectatorInputContextActive(bool bActive);
	
	bool IsHitReactInputBlocked() const;

	void HandleAbilityInputPressed(EBlackoutAbilityInputID InputID);
	void HandleAbilityInputReleased(EBlackoutAbilityInputID InputID);
	void TryInitHUD() const;
	UBlackoutAbilitySystemComponent* GetBlackoutAbilitySystemComponent() const;
	UBlackoutCombatComponent* GetBlackoutCombatComponent() const;

#pragma endregion 
	
};
