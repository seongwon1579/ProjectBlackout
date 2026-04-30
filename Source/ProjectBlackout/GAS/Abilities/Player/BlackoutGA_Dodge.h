#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Dodge.generated.h"

class UAnimMontage;

/**
 * 방향 입력에 따라 구르기 또는 백스텝을 수행하는 플레이어 회피 GA.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Dodge : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Dodge();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float StaminaCost = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float DodgeStrength = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float BackstepStrength = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float DodgeDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float UpwardImpulse = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	TObjectPtr<UAnimMontage> DodgeMontage;
	
	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	TObjectPtr<UAnimMontage> DodgeMontage2R;
	*/

private:
	UFUNCTION()
	void OnDodgeFinished();

	bool ConsumeStamina() const;
	FVector CalculateDodgeDirection(const FGameplayAbilityActorInfo* ActorInfo, bool& bOutIsBackstep) const;

	FTimerHandle DodgeEndTimerHandle;
};
