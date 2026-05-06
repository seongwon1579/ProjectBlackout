// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlackoutMinionGameplayAbility.h"
#include "Templates/SubclassOf.h"
#include "GA_Wraith_FireTwinArrows.generated.h"

class ABOProjectile;
class UAnimMontage;

/**
 * Wraith 2연발 화살.
 * 사거리 도달 + 시야 확보 시 발동. 발사 완료 직후 Teleport 연계.
 */
UCLASS()
class PROJECTBLACKOUT_API
	UGA_Wraith_FireTwinArrows : public UBlackoutMinionGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Wraith_FireTwinArrows();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                             const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo
	                             ActivationInfo,
	                             const FGameplayEventData*
	                             TriggerEventData) override;

	UFUNCTION()
	void OnAimDelayFinished();

	UFUNCTION()
	void OnSecondShotDelayFinished();

	void FireOneArrow();

	UPROPERTY(EditDefaultsOnly, Category = "Wraith")
	TSubclassOf<ABOProjectile> ArrowProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith", meta = (ClampMin = 0.0f))
	float AimDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith", meta = (ClampMin = 0.0f))
	float SecondShotDelay = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith", meta = (ClampMin = 0.0f))
	float DamageMagnitude = 12.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Wraith")
	TObjectPtr<UAnimMontage> BowshotMontage;
};
