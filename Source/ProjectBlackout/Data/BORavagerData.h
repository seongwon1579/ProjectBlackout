// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "BORavagerData.generated.h"

/**
 * 
 */
class ABOEnemySpawnerProjectile;
class ABOEnemyProjectile; 

USTRUCT()
struct FBossMeleeSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> Effect;
	
	UPROPERTY(EditAnywhere)
	TArray<FName> HitboxComponentNames;
	
	UPROPERTY(EditAnywhere)
	float DamageMagnitude = 10.f;
	
	bool IsValid() const { return Effect && DamageMagnitude > 0.f && HitboxComponentNames.Num() > 0; }
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
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	float DamageTickInterval = 0.5f;
	
	UPROPERTY(EditAnywhere)
	float AbilityLevel = 1.f;
	
	bool IsValid() const;
};

UCLASS()
class PROJECTBLACKOUT_API UBORavagerData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	TSubclassOf<UGameplayAbility> GrantedAbility;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Anim",meta = (Categories = "Ability"))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Montages;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Melee")
	FBossMeleeSettings MeleeSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Projectile")
	FBossProjectileSettings ProjectileSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Minion")
	FBossMinionSpawnSettings MinionSettings;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Gorenado")
	FBossGorenadoSettings GorenadoSettings;
	
};
