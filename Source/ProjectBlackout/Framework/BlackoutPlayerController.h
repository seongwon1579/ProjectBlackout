#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UBlackoutAbilitySystemComponent;

UCLASS()
class PROJECTBLACKOUT_API ABlackoutPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
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

	UFUNCTION(BlueprintCallable, Category = "Blackout|Controller")
	void EnterSpectatorMode();
	
#pragma region InputSetup
protected:
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> MeleeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> SprintAction;

	void OnFirePressed();
	void OnFireReleased();
	void OnReloadPressed();
	void OnMeleePressed();
	void OnDodgePressed();
	void OnSprintPressed();
	void OnSprintReleased();

	void HandleAbilityInputPressed(EBlackoutAbilityInputID InputID);
	void HandleAbilityInputReleased(EBlackoutAbilityInputID InputID);
	UBlackoutAbilitySystemComponent* GetBlackoutAbilitySystemComponent() const;

#pragma endregion 
	
};
