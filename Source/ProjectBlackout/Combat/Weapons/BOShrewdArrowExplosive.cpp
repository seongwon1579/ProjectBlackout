// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Weapons/BOShrewdArrowExplosive.h"

#include "BlackoutDamageable.h"
#include "BlackoutHitboxComponent.h"
#include "BlackoutLog.h"
#include "BlackoutWeaponCueLibrary.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h" 
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"

ABOShrewdArrowExplosive::ABOShrewdArrowExplosive()
{
	ExplosionCueTag = BlackoutGameplayTags::GameplayCue_Shrewd_Projectile_Explosion;
}

void ABOShrewdArrowExplosive::Launch(const FVector& Velocity)
{
	if (!Movement) return;

	IgnoreFirerWhenMoving();

	Movement->Velocity = Velocity;
	Movement->SetActive(true, true);
	
	if (HasAuthority())
	{
		++ReplicatedNetState.StateId;
		ReplicatedNetState.bActive = true;
		ReplicatedNetState.Location = GetActorLocation();
		ReplicatedNetState.Direction = Velocity.GetSafeNormal();
		ReplicatedNetState.Speed = Velocity.Size();
		ReplicatedNetState.GravityScale = Movement->ProjectileGravityScale;
		ApplyProjectileNetState();
		ForceNetUpdate();
	}
}

void ABOShrewdArrowExplosive::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == nullptr || OtherActor == this
		|| OtherActor == GetInstigator() || OtherActor == GetOwner())
	{
		return;
	}

	if (HasAuthority())
	{
		ApplyImpactDamage(Hit);
		ExecuteExplosionCue(Hit);
	}

	ReturnToPool();
}

void ABOShrewdArrowExplosive::ExecuteExplosionCue(const FHitResult& Hit)
{
	if (!Hit.bBlockingHit) return;
	if (!ExplosionCueTag.IsValid())
	{
		BO_LOG_CORE(Warning, "ExecuteExplosionCue skipped: ExplosionCueTag가 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	FGameplayCueParameters Params = UBlackoutWeaponCueLibrary::BuildImpactCueParameters(
		this, Hit);
	ExecuteProjectileGameplayCue(ExplosionCueTag, Params);
}

void ABOShrewdArrowExplosive::ApplyImpactDamage(const FHitResult& Hit)
{
	if (!DamageSpec.IsValid()) return;

	const FVector ExplosionLocation = Hit.ImpactPoint;
	
#if ENABLE_DRAW_DEBUG
	if (bShowDebugExplosion)
	{
		DrawDebugSphere(
			GetWorld(),
			ExplosionLocation,
			ExplosionRadius,
			24,                
			FColor::Red,
			false,              
			2.0f,               
			0,                   
			2.0f              
		);
	}
#endif

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ExplosiveOverlap), false, this);
	Params.AddIgnoredActor(this);
	if (AActor* Firer = GetInstigator())
	{
		Params.AddIgnoredActor(Firer);
	}

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		ExplosionLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ExplosionRadius),
		Params);

	TSet<AActor*> Damaged;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || Damaged.Contains(HitActor)) continue;
		Damaged.Add(HitActor);

		if (UBlackoutHitboxComponent* Hitbox = Cast<UBlackoutHitboxComponent>(Overlap.GetComponent()))
		{
			Hitbox->ReceiveDamageSpec(DamageSpec);
		}
		else if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor))
		{
			Damageable->ReceiveDamageFromHitbox(DamageSpec, NAME_None);
		}
	}
}
