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

	virtual void InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius);
	virtual void Launch(const FVector& Direction);

protected:
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ReturnToPool();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float SplashRadius = 0.0f;

	FGameplayEffectSpecHandle DamageSpec;
};
