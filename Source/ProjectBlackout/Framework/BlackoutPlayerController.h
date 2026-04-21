#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "BlackoutPlayerController.generated.h"

class UInputMappingContext;
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
public:
	virtual void SetupInputComponent() override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;
	
	
	
#pragma endregion 
	
};
