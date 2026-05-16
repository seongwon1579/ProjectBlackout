// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Weapons/BOShockwaveProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BlackoutDamageable.h"
#include "BlackoutGameplayTags.h"
#include "NiagaraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABOShockwaveProjectile::ABOShockwaveProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Collision"));
	CollisionComp->SetCollisionProfileName(TEXT("EnemyProjectile")); 
	CollisionComp->OnComponentHit.AddDynamic(this, &ABOShockwaveProjectile::OnHit);
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

void ABOShockwaveProjectile::InitializeProjectile(const FProjectileSpawnParams& InSpawnParams)
{
	SpawnParams = InSpawnParams;
	
	if (InSpawnParams.Speed >= 0.f)
	{
		ProjectileMovement->InitialSpeed = InSpawnParams.Speed;
		ProjectileMovement->MaxSpeed = InSpawnParams.Speed;
	}
	
	if (InSpawnParams.LifeSpan > 0.f)
	{
		SetLifeSpan(InSpawnParams.LifeSpan);
	}
}

void ABOShockwaveProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetInstigator()) return;
	
	if (!SpawnParams.Effect) { Destroy(); return; }
	
	IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(OtherActor);
	if (!Damageable) { Destroy(); return; }
	
	UAbilitySystemComponent* OwnerASC = 
	UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!OwnerASC) { Destroy(); return; }
	
	FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
	EffectContext.AddSourceObject(GetInstigator());
    
	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(
		SpawnParams.Effect, SpawnParams.AbilityLevel, EffectContext);
    
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(
			BlackoutGameplayTags::Data_Damage, 
			SpawnParams.DamageMagnitude);
        
		Damageable->ReceiveDamageFromHitbox(SpecHandle, Hit.BoneName);
	}
    
	Destroy();
}


