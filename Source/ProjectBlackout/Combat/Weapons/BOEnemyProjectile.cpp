// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Weapons/BOEnemyProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BlackoutDamageable.h"
#include "BlackoutGameplayTags.h"
#include "NiagaraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABOEnemyProjectile::ABOEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	
	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	CollisionComp->SetCollisionProfileName(TEXT("EnemyProjectile")); 
	RootComponent = CollisionComp;
    
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
    
	Effect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
	Effect->SetupAttachment(RootComponent);
    
	InitialLifeSpan = 5.f;  
}

void ABOEnemyProjectile::InitializeProjectile(const FProjectileSpawnData& InSpawnParams)
{
	SpawnParams = InSpawnParams;
	
	if (InSpawnParams.Speed > 0.f)
	{
		ProjectileMovement->InitialSpeed = InSpawnParams.Speed;
		ProjectileMovement->MaxSpeed = InSpawnParams.Speed;
		
		ProjectileMovement->Velocity = GetActorForwardVector() * InSpawnParams.Speed;
	}
	
	if (InSpawnParams.LifeSpan > 0.f)
	{
		SetLifeSpan(InSpawnParams.LifeSpan);
	}
}

void ABOEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (CollisionComp)
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &ABOEnemyProjectile::OnHit);
	}
	
	if (!HasAuthority() && ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}
}

void ABOEnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShouldIgnoreHit(OtherActor)) return;
	
	if (!HasAuthority()) return;
	
	ApplyDamageToTarget(OtherActor, Hit.BoneName);
	Destroy();
	
}

void ABOEnemyProjectile::ApplyDamageToTarget(AActor* Target, FName HitBoneName)
{
	if (!SpawnParams.Effect) return;
	
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Target);
	if (!Damageable) return;
	
	UAbilitySystemComponent* OwnerASC = 
	UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!OwnerASC) return;
	
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetInstigator());
    
	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(
		SpawnParams.Effect, SpawnParams.AbilityLevel, EffectContext);
    
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			BlackoutGameplayTags::Data_Damage, 
			SpawnParams.DamageMagnitude);
        
		Damageable->ReceiveDamageFromHitbox(SpecHandle, HitBoneName);
	}
}

bool ABOEnemyProjectile::ShouldIgnoreHit(AActor* OtherActor) const
{
	if (!OtherActor) return true;
	
	if (OtherActor == GetInstigator()) return true;
	
	if (OtherActor == GetOwner()) return true;

	return false;
}


