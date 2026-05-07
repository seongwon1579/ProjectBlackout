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
class UBOCharacterData;
class UBOConsumableData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutConsumableCountsChangedSignature, int32, BloodRootCount, int32, GulSerumCount);

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

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState|Consumables")
	void SetConsumableCounts(int32 NewBloodRootCount, int32 NewGulSerumCount);

	void InitializeConsumablesFromCharacterData(const UBOCharacterData* CharacterData);

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|Consumables")
	int32 GetConsumableCount(FGameplayTag ConsumableTag) const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState|Consumables")
	bool SetConsumableCount(FGameplayTag ConsumableTag, int32 NewCount);

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState|Consumables")
	bool ConsumeConsumable(FGameplayTag ConsumableTag, int32 Amount = 1);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|PlayerState")
	FGameplayTag SelectedClassTag;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BloodRootCount, Category = "Blackout|PlayerState")
	int32 BloodRootCount = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GulSerumCount, Category = "Blackout|PlayerState")
	int32 GulSerumCount = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|PlayerState")
	bool bIsReady = false;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|PlayerState|Consumables")
	FBlackoutConsumableCountsChangedSignature OnConsumableCountsChanged;

protected:
	UFUNCTION()
	void OnRep_BloodRootCount();

	UFUNCTION()
	void OnRep_GulSerumCount();

	void BroadcastConsumableCounts();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|GAS")
	TObjectPtr<UBlackoutAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UBlackoutBaseAttributeSet> BaseAttributeSet;

	UPROPERTY()
	TObjectPtr<const UBlackoutPlayerAttributeSet> PlayerAttributeSet;

	UPROPERTY()
	TObjectPtr<const UBlackoutAmmoAttributeSet> AmmoAttributeSet;

};
