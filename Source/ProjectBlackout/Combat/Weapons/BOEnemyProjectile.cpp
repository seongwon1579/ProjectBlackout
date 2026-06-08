// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Weapons/BOEnemyProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BlackoutDamageable.h"
#include "BlackoutGameplayTags.h"
#include "BlackoutPlayerCharacter.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABOEnemyProjectile::ABOEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComp->SetCollisionProfileName(TEXT("EnemyProjectile"));
	CollisionComp->SetBoxExtent(FVector(50.f, 150.f, 30.f)); 
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
	
	SetCollisionEvent();
}

void ABOEnemyProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldIgnoreHit(OtherActor)) return;
	if (!HasAuthority()) return;

	ApplyDamageToTarget(OtherActor, SweepResult.BoneName);
	Destroy();
}

void ABOEnemyProjectile::ApplyDamageToTarget(AActor* Target, FName HitBoneName)
{
	if (!SpawnParams.Effect) return;
	
	if (!Target || !Target->IsA(ABlackoutPlayerCharacter::StaticClass())) return;
	
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

void ABOEnemyProjectile::SetCollisionEvent()
{
	if (CollisionComp)
	{
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ABOEnemyProjectile::OnBeginOverlap);
	}
	
	if (!HasAuthority() && ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}
	
}


