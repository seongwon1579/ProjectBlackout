#include "Combat/Weapons/BOProjectile.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interfaces/BlackoutDamageable.h"
#include "Net/UnrealNetwork.h"
#include "Pool/BlackoutPoolSubsystem.h"

ABOProjectile::ABOProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);
	SetNetDormancy(DORM_Awake);
	SetNetUpdateFrequency(60.f);
	SetMinNetUpdateFrequency(30.f);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;
	Collision->OnComponentHit.AddDynamic(this, &ABOProjectile::OnHit);

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->bAutoActivate = false;
}

void ABOProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABOProjectile, ReplicatedNetState);
}

void ABOProjectile::OnSpawnFromPool_Implementation()
{
	if (HasAuthority())
	{
		SetNetDormancy(DORM_Awake);
		FlushNetDormancy();
	}

	ApplyActiveState(true);
	Movement->Velocity = FVector::ZeroVector;
}

void ABOProjectile::BeginPlay()
{
	Super::BeginPlay();

	ApplyProjectileNetState();
}

void ABOProjectile::OnReturnToPool_Implementation()
{
	SetActorHiddenInGame(true);
	if (HasAuthority())
	{
		++ReplicatedNetState.StateId;
		ReplicatedNetState.bActive = false;
	}

	ApplyActiveState(false);
	DamageSpec = FGameplayEffectSpecHandle();

	if (HasAuthority())
	{
		ForceNetUpdate();
		SetNetDormancy(DORM_DormantAll);
	}
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

	if (HasAuthority())
	{
		SetNetDormancy(DORM_Awake);
		FlushNetDormancy();
		++ReplicatedNetState.StateId;
		ReplicatedNetState.bActive = true;
		ReplicatedNetState.Location = GetActorLocation();
		ReplicatedNetState.Direction = Direction.GetSafeNormal();
		ReplicatedNetState.Speed = Movement->InitialSpeed;
		ApplyProjectileNetState();
		ForceNetUpdate();
	}
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

float ABOProjectile::GetImpactFuseArmDistance() const
{
	return 0.0f;
}

void ABOProjectile::OnRep_ProjectileNetState()
{
	ApplyProjectileNetState();
}

void ABOProjectile::ApplyProjectileNetState()
{
	if (!HasAuthority() && ReplicatedNetState.StateId < LastAppliedStateId)
	{
		return;
	}

	LastAppliedStateId = ReplicatedNetState.StateId;
	ApplyActiveState(ReplicatedNetState.bActive);

	if (HasAuthority() || !ReplicatedNetState.bActive || !Movement || ReplicatedNetState.Speed <= 0.0f)
	{
		return;
	}

	SetActorLocationAndRotation(
		ReplicatedNetState.Location,
		ReplicatedNetState.Direction.Rotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	Movement->Velocity = FVector(ReplicatedNetState.Direction) * ReplicatedNetState.Speed;
	Movement->SetActive(true, true);
}

void ABOProjectile::ApplyActiveState(bool bIsActive)
{
	SetActorHiddenInGame(!bIsActive);

	if (Collision)
	{
		Collision->SetCollisionEnabled(bIsActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	if (!bIsActive && Movement)
	{
		Movement->Deactivate();
		Movement->Velocity = FVector::ZeroVector;
	}
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
	if (!HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (UBlackoutPoolSubsystem* Pool = World->GetSubsystem<UBlackoutPoolSubsystem>())
		{
			Pool->ReturnToPool(this);
		}
	}
}
