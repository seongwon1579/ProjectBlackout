#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GA_FireWeapon.generated.h"

class UGameplayEffect;

/**
 * 플레이어 사격 게임플레이 어빌리티 (TDD v5 §4.1)
 * 코스트 지불(탄약), 몽타주 재생, 트레이스/발사체 스폰을 처리합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_FireWeapon : public UBlackoutGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_FireWeapon();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float ParallaxMaxDistance = 10000.0f;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FHitResult PerformTrace(const FVector& Start, const FVector& End);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayEffectSpecHandle BuildDamageSpec(const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool ApplyAmmoCost();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void PlayFireMontage();
};
