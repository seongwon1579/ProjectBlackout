#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOProjectile.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "BOMeridianGrenadeProjectile.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

#if WITH_EDITOR
struct FPropertyChangedEvent;
#endif

UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABOMeridianGrenadeProjectile : public ABOProjectile
{
	GENERATED_BODY()

public:
	ABOMeridianGrenadeProjectile();

	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnSpawnFromPool_Implementation() override;
	virtual void OnReturnToPool_Implementation() override;
	virtual void InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius) override;
	virtual void Launch(const FVector& Direction) override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void SetProjectileMesh(UStaticMesh* InMesh);

	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	bool IsFuseArmed() const { return bFuseArmed; }

protected:
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	void ApplyProjectileSettingsToComponents();
	void ResetGrenadeState();
	void UpdateFuseState();
	FGameplayEffectSpecHandle MakeImpactDamageSpec(const FGameplayEffectSpecHandle& SourceSpec) const;
	void ApplyImpactDamage(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit);
	void Explode(const FHitResult& Hit);
	void ApplyExplosionDamage(const FVector& Origin);
	void ExecuteExplosionCue(const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = "0.0", Units = "cm"))
	float ArmDistance = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = "0.0"))
	float ImpactDamageMultiplier = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = "0.0", Units = "cm"))
	float CollisionRadius = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Visual", meta = (ClampMin = "0.0"))
	float MeshScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float GravityScale = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Bounciness = 0.45f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Friction = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = "0.0", Units = "s"))
	float AutoReturnDelay = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FGameplayTag ExplosionCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug")
	bool bDrawDebugExplosionRadius = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug", meta = (EditCondition = "bDrawDebugExplosionRadius", ClampMin = "0.0"))
	float DebugExplosionRadiusDuration = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug", meta = (EditCondition = "bDrawDebugExplosionRadius", ClampMin = "4"))
	int32 DebugExplosionRadiusSegments = 24;

	FGameplayEffectSpecHandle ExplosionDamageSpec;
	FVector PreviousLocation = FVector::ZeroVector;
	FTimerHandle AutoReturnTimerHandle;
	float TraveledDistance = 0.0f;
	bool bFuseArmed = false;
	bool bExploded = false;

	void StartAutoReturnTimer();
	void ClearAutoReturnTimer();
	void ReturnToPoolByLifetime();
};
