#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/BlackoutWeaponStat.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BOProjectile.generated.h"

class UAbilitySystemComponent;
class UProjectileMovementComponent;
class USceneComponent;
class USphereComponent;

USTRUCT()
struct FBOProjectileNetState
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 StateId = 0;

	UPROPERTY()
	bool bActive = false;

	UPROPERTY()
	FVector_NetQuantize10 Location = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantizeNormal Direction = FVector::ForwardVector;

	UPROPERTY()
	float Speed = 0.0f;
	
	UPROPERTY()
	float GravityScale = 1.0f;  
};

UCLASS()
class PROJECTBLACKOUT_API ABOProjectile : public AActor, public IBlackoutPoolableInterface
{
	GENERATED_BODY()
	
public:	
	ABOProjectile();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnSpawnFromPool_Implementation() override;
	virtual void OnReturnToPool_Implementation() override;

	virtual void InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius);
	virtual void InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius, const FBlackoutWeaponCueSet& InCueSet);
	virtual void Launch(const FVector& Direction);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetInitialSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetGravityScale() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetCollisionRadius() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	virtual float GetImpactFuseArmDistance() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_ProjectileNetState();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void ApplyProjectileNetState();
	void ApplyActiveState(bool bIsActive);
	// 발사자(firer) 콜리전을 이동 sweep에서 무시하도록 설정. 서버 Launch와 클라 NetState 적용 양쪽에서 사용.
	void IgnoreFirerWhenMoving();
	void ExecuteImpactCue(const FHitResult& Hit) const;
	UAbilitySystemComponent* GetCueAbilitySystemComponent() const;
	void ReturnToPool();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UProjectileMovementComponent> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Visual")
	TObjectPtr<USceneComponent> TrailCueAttachComponent;

	// 비행 중 적용할 루핑 트레일 Gameplay Cue 태그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Visual")
	FGameplayTag TrailCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Visual")
	FVector TrailCueLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	float SplashRadius = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileNetState)
	FBOProjectileNetState ReplicatedNetState;

	uint32 LastAppliedStateId = 0;

	FGameplayEffectSpecHandle DamageSpec;
	FBlackoutWeaponCueSet CueSet;
};
