// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "BORavagerPatternData.generated.h"

/**
 * 
 */
class ABOEnemySpawnerProjectile;
class ABOEnemyProjectile; 

USTRUCT()
struct FBossBasicAttackSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> Effect;
	
	UPROPERTY(EditAnywhere)
	TArray<FName> HitboxComponentNames;
	
	UPROPERTY(EditAnywhere)
	float DamageMagnitude = 10.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.0f;
	
	bool IsValid() const { return Effect && (DamageMagnitude > 0.f || StunMagnitude > 0.f) && HitboxComponentNames.Num() > 0; }
};


USTRUCT()
struct FMinionSpawnData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ACharacter> MinionClass;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.1"))
	float HatchDelay = 1.5f;
    
	UPROPERTY(EditAnywhere)
	int32 MinionLevel = 1;
	
};

USTRUCT()
struct FProjectileSpawnData
{
	GENERATED_BODY()
	 
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> Effect;
	
	UPROPERTY(EditAnywhere)
	float AbilityLevel = 1.f;
	
	UPROPERTY(EditAnywhere)
	float LifeSpan = 5.f;
	
	UPROPERTY(EditAnywhere)
	float Speed = 1300.f;
	
	UPROPERTY(EditAnywhere)
	float DamageMagnitude = 10.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.0f;
};

USTRUCT()
struct FBossProjectileSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FName SocketName = NAME_None;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABOEnemyProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere)
	FProjectileSpawnData ProjectileSpawnData;
	
	bool IsValid() const;
};

USTRUCT()
struct FBossMinionSpawnSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABOEnemySpawnerProjectile> SpawnerClass;
	
	UPROPERTY(EditAnywhere)
	FName SocketName = NAME_None;
	
	UPROPERTY(EditAnywhere)
	FProjectileSpawnData ProjectileSpawnData;
	
	UPROPERTY(EditAnywhere)
	FMinionSpawnData MinionSpawnData;
	
	UPROPERTY(EditAnywhere)
	int32 SpawnCount = 3;
	
	UPROPERTY(EditAnywhere)
	float SpreadAngle = 30.f;
    
	UPROPERTY(EditAnywhere)
	float ThrowPitch = 45.f;
	
	UPROPERTY(EditAnywhere, Category = "Spawn|Throw")
	float ThrowPitchVariance = 10.f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Throw", meta = (ClampMin = "0.1"))
	float DistanceScaleMin = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Spawn|Throw", meta = (ClampMin = "0.1"))
	float DistanceScaleMax = 1.3f;
    
	UPROPERTY(EditAnywhere, Category = "Elite")
	FMinionSpawnData EliteMinionSpawnData;

	UPROPERTY(EditAnywhere, Category = "Elite")
	int32 EliteSpawnCount = 1;

	UPROPERTY(EditAnywhere, Category = "Elite")
	float EliteSpawnRadius = 500.f;

	bool IsValid() const;
	
};

USTRUCT(BlueprintType)
struct FPullData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	FVector PullDirection = FVector::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	float PullStrength = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	float DeltaTime = 0.f;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;
	
};

USTRUCT()
struct FBossGorenadoSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float PullRadius = 3000.f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float DamageRadius = 500.f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float PullStrength = 800.f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float MinDistance = 300.f;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY(EditAnywhere)
	FVector PullDirection = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere)
	float Damage = 15.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.0f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float DamageTickInterval = 0.5f;
	
	UPROPERTY(EditAnywhere)
	float AbilityLevel = 1.f;
	
	bool IsValid() const;
};

USTRUCT()
struct FBossEnergyBurstSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, meta = (ClampMin = "100"))
	float DamageRadius = 800.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY(EditAnywhere)
	float Damage = 999999.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.0f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float DamageTickInterval = 0.5f;

	UPROPERTY(EditAnywhere)
	float AbilityLevel = 1.f;
    
	bool IsValid() const { return DamageEffect != nullptr; }
};

USTRUCT(BlueprintType)
struct FBossChargeSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float DamageMagnitude = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float StunMagnitude = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hitbox")
	TArray<FName> HitboxComponentNames;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	FName EndSectionName = FName("End");
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float ChargeSpeed = 1500.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic", meta = (ClampMin = "0.0"))
	float ChargeStopDistance = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic", meta = (ClampMin = "0.001"))
	float CheckInterval = 0.02f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic", meta = (ClampMin = "0.0"))
	float MaxChargeDuration = 3.f;

	bool IsValid() const { return DamageEffect != nullptr && (DamageMagnitude > 0.f || StunMagnitude > 0.f) && HitboxComponentNames.Num() > 0; }
};

UCLASS()
class PROJECTBLACKOUT_API UBORavagerPatternData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	TSubclassOf<UGameplayAbility> GrantedAbility;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Anim",meta = (Categories = "Ability"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Montages;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Chase")
	float ChaseStartRange = 800.f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Chase")
	float ChaseEndRange = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Chase")
	float AttackRange = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Chase", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AttackRangeVariance = 0.f;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Melee")
	FBossBasicAttackSettings BasicAttackSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Projectile")
	FBossProjectileSettings ProjectileSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Minion")
	FBossMinionSpawnSettings MinionSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Gorenado")
	FBossGorenadoSettings GorenadoSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|EnergyBurst")
	FBossEnergyBurstSettings EnergyBurstSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Charge")
	FBossChargeSettings ChargeSettings;
};
