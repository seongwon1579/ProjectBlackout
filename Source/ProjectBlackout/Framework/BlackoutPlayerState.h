#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutPlayerState.generated.h"

class UBlackoutAmmoAttributeSet;
class UBlackoutPlayerAttributeSet;
class UBlackoutBaseAttributeSet;
class UBlackoutAbilitySystemComponent;

UCLASS()
class PROJECTBLACKOUT_API ABlackoutPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABlackoutPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UBlackoutAbilitySystemComponent* GetBlackoutAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState")
	void ApplyBattleTransitionPolicy(EBattleTransitionType TransitionType);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|PlayerState")
	FGameplayTag SelectedClassTag;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|PlayerState")
	int32 BloodRootCount = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|PlayerState")
	int32 GulSerumCount = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|PlayerState")
	bool bIsReady = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|GAS")
	TObjectPtr<UBlackoutAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UBlackoutBaseAttributeSet> BaseAttributeSet;

	UPROPERTY()
	TObjectPtr<const UBlackoutPlayerAttributeSet> PlayerAttributeSet;

	UPROPERTY()
	TObjectPtr<const UBlackoutAmmoAttributeSet> AmmoAttributeSet;

};
