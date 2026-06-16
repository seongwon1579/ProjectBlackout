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
#include "Pool/BlackoutPoolSubsystem.h"

ABOEnemyProjectile::ABOEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	
	bReplicates = true;
	
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComp->SetCollisionProfileName(TEXT("EnemyProjectile"));
	RootComponent = CollisionComp;
    
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	ProjectileMovement->InitialSpeed = 1500.f;
	ProjectileMovement->MaxSpeed = 1500.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
    
	Effect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
	Effect->SetupAttachment(RootComponent);

	// 풀 재사용 시 수명 타이머는 InitializeProjectile에서 서버가 다시 무장합니다.
	InitialLifeSpan = 0.f;
	DefaultCollisionEnabled = CollisionComp->GetCollisionEnabled();
}

void ABOEnemyProjectile::OnSpawnFromPool_Implementation()
{
	bReturnedToPool = false;

	SetLifeSpan(0.0f);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (CollisionComp)
	{
		// 소유자/속도 설정이 끝나기 전 조기 오버랩을 막기 위해 InitializeProjectile에서 다시 켭니다.
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}
}

void ABOEnemyProjectile::OnReturnToPool_Implementation()
{
	SetLifeSpan(0.0f);
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}

	SpawnParams = FProjectileSpawnData();
	SetOwner(nullptr);
	SetInstigator(nullptr);
}

void ABOEnemyProjectile::InitializeProjectile(const FProjectileSpawnData& InSpawnParams)
{
	bReturnedToPool = false;
	SpawnParams = InSpawnParams;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	
	if (InSpawnParams.Speed > 0.f)
	{
		ProjectileMovement->InitialSpeed = InSpawnParams.Speed;
		ProjectileMovement->MaxSpeed = InSpawnParams.Speed;
		
		ProjectileMovement->Velocity = GetActorForwardVector() * InSpawnParams.Speed;
	}

	if (CollisionComp)
	{
		CollisionComp->SetCollisionEnabled(DefaultCollisionEnabled);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->UpdateComponentVelocity();
		ProjectileMovement->Activate(true);
	}
	
	if (InSpawnParams.LifeSpan > 0.f)
	{
		SetLifeSpan(InSpawnParams.LifeSpan);
	}
	else
	{
		SetLifeSpan(0.0f);
	}
}

void ABOEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority() && ProjectileMovement)
	{
		ProjectileMovement->SetActive(false);
	}
	
	SetCollisionEvent();
}

void ABOEnemyProjectile::LifeSpanExpired()
{
	SetLifeSpan(0.0f);

	if (HasAuthority())
	{
		ReturnToPool();
	}
}

void ABOEnemyProjectile::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ShouldIgnoreHit(OtherActor)) return;
	if (!HasAuthority()) return;

	ApplyDamageToTarget(OtherActor, SweepResult.BoneName);
	ReturnToPool();
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
		SpecHandle.Data->SetSetByCallerMagnitude(
			BlackoutGameplayTags::Data_Stun,
			SpawnParams.StunMagnitude);
        
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
}

void ABOEnemyProjectile::ReturnToPool()
{
	if (!HasAuthority() || bReturnedToPool)
	{
		return;
	}

	bReturnedToPool = true;

	if (UWorld* World = GetWorld())
	{
		if (UBlackoutPoolSubsystem* PoolSubsystem = World->GetSubsystem<UBlackoutPoolSubsystem>())
		{
			PoolSubsystem->ReturnToPool(this);
			return;
		}
	}

	Destroy();
}
