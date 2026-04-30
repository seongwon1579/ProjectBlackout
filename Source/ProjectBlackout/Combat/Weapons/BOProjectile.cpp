#include "Combat/Weapons/BOProjectile.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/BlackoutDamageable.h"
#include "Pool/BlackoutPoolSubsystem.h"

ABOProjectile::ABOProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;
	Collision->OnComponentHit.AddDynamic(this, &ABOProjectile::OnHit);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->bAutoActivate = false;
}

void ABOProjectile::OnSpawnFromPool_Implementation()
{
	SetActorHiddenInGame(false);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Movement->Velocity = FVector::ZeroVector;
}

void ABOProjectile::OnReturnToPool_Implementation()
{
	SetActorHiddenInGame(true);
	Movement->Deactivate();
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageSpec = FGameplayEffectSpecHandle();
}

void ABOProjectile::InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius)
{
	DamageSpec = InDamageSpec;
	SplashRadius = Radius;
}

void ABOProjectile::Launch(const FVector& Direction)
{
	if (!Movement)
	{
		return;
	}

	Movement->Velocity = Direction.GetSafeNormal() * Movement->InitialSpeed;
	Movement->SetActive(true, true);
}

float ABOProjectile::GetInitialSpeed() const
{
	return Movement ? Movement->InitialSpeed : 0.0f;
}

float ABOProjectile::GetGravityScale() const
{
	return Movement ? Movement->ProjectileGravityScale : 1.0f;
}

float ABOProjectile::GetCollisionRadius() const
{
	return Collision ? Collision->GetScaledSphereRadius() : 0.0f;
}

void ABOProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HasAuthority() && DamageSpec.IsValid())
	{
		if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(OtherComp))
		{
			HitboxComponent->ReceiveDamageSpec(DamageSpec);
		}
		else if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(OtherActor))
		{
			Damageable->ReceiveDamageFromHitbox(DamageSpec, Hit.BoneName);
		}
	}

	ReturnToPool();
}

void ABOProjectile::ReturnToPool()
{
	if (UWorld* World = GetWorld())
	{
		if (UBlackoutPoolSubsystem* Pool = World->GetSubsystem<UBlackoutPoolSubsystem>())
		{
			Pool->ReturnToPool(this);
		}
	}
}
