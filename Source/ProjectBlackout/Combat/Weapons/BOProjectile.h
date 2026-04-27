#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BOProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PROJECTBLACKOUT_API ABOProjectile : public AActor, public IBlackoutPoolableInterface
{
	GENERATED_BODY()
	
public:	
	ABOProjectile();

	virtual void OnSpawnFromPool_Implementation() override;
	virtual void OnReturnToPool_Implementation() override;

	void InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius);
	void Launch(const FVector& Direction);

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float SplashRadius = 0.0f;

	FGameplayEffectSpecHandle DamageSpec;
};
