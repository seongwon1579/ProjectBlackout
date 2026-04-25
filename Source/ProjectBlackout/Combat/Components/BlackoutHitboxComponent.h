#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "BlackoutHitboxComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutHitboxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:	
	UBlackoutHitboxComponent();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayTag GetPartTag() const { return PartTag; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetDamageMultiplier() const { return DamageMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void ReceiveDamageSpec(const FGameplayEffectSpecHandle& SpecHandle);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	FGameplayTag PartTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	FName AttachedBoneName;
};
