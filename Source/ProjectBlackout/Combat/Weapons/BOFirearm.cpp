#include "Combat/Weapons/BOFirearm.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/Weapons/BOProjectile.h"
#include "Core/BlackoutLog.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "Interfaces/BlackoutDamageable.h"
#include "Pool/BlackoutPoolSubsystem.h"

namespace
{
	bool TryGetHealthForDebug(const AActor* TargetActor, float& OutHealth)
	{
		const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(TargetActor);
		const UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
		if (!AbilitySystemComponent)
		{
			return false;
		}

		OutHealth = AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
		return true;
	}
}

ABOFirearm::ABOFirearm()
{
	MuzzleFlash = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MuzzleFlash"));
	MuzzleFlash->SetupAttachment(WeaponMesh);
	MuzzleFlash->SetAutoActivate(false);
	
	MuzzleSocket = TEXT("MuzzleSocket");
}

bool ABOFirearm::InitializeStatsFromDataTable()
{
	Super::InitializeStatsFromDataTable();

	if (const FBlackoutFirearmStat* FoundStats = FirearmStatsRow.GetRow<FBlackoutFirearmStat>(TEXT("BOFirearm::InitializeStatsFromDataTable")))
	{
		CachedFirearmStats = *FoundStats;
		ApplyCommonStats(CachedFirearmStats);
		return true;
	}

	return false;
}

FHitResult ABOFirearm::Fire(const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	FHitResult HitResult;
	if (bUseHitscan)
	{
		if (UWorld* World = GetWorld())
		{
			const FVector TraceStart = GetMuzzleTransform().GetLocation();
			const FVector TraceEnd = TraceStart + Direction.GetSafeNormal() * 10000.0f;

			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BOFirearm_Fire), false, GetOwner());
			QueryParams.AddIgnoredActor(this);

			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
			AActor* HitActor = HitResult.GetActor();
			UPrimitiveComponent* HitComponent = HitResult.GetComponent();
			AActor* DamageTargetActor = HitActor;

			if (bDrawDebugHitscanRay)
			{
				const bool bHit = HitResult.bBlockingHit;
				const FVector DebugEnd = bHit ? HitResult.ImpactPoint : TraceEnd;
				const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
				DrawDebugLine(World, TraceStart, DebugEnd, DebugColor, false, DebugHitscanRayDuration, 0, DebugHitscanRayThickness);
			}

			float HealthBefore = 0.0f;
			bool bCapturedHealthBefore = bDrawDebugHitscanRay && TryGetHealthForDebug(DamageTargetActor, HealthBefore);
			bool bAppliedDamage = false;

			if (HasAuthority() && HitResult.bBlockingHit && DamageSpecHandle.IsValid())
			{
				if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitComponent))
				{
					DamageTargetActor = HitboxComponent->GetOwner();
					if (bDrawDebugHitscanRay && !bCapturedHealthBefore)
					{
						bCapturedHealthBefore = TryGetHealthForDebug(DamageTargetActor, HealthBefore);
					}
					HitboxComponent->ReceiveDamageSpec(DamageSpecHandle);
					bAppliedDamage = true;
				}
				else if (HitActor)
				{
					if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor))
					{
						Damageable->ReceiveDamageFromHitbox(DamageSpecHandle, HitResult.BoneName);
						bAppliedDamage = true;
					}
				}
			}

			if (bDrawDebugHitscanRay)
			{
				float HealthAfter = 0.0f;
				const bool bCapturedHealthAfter = bAppliedDamage && TryGetHealthForDebug(DamageTargetActor, HealthAfter);
				FString DamageText = TEXT("None");
				if (bCapturedHealthBefore && bCapturedHealthAfter)
				{
					const float DamageDealt = FMath::Max(HealthBefore - HealthAfter, 0.0f);
					DamageText = FString::Printf(TEXT("%.1f"), DamageDealt);
				}
				else if (bAppliedDamage)
				{
					DamageText = TEXT("Unknown");
				}

				BO_SCREEN_CORE(Log,
				               "Hitscan Debug: Target=%s Component=%s Damage=%s",
				               *GetNameSafe(HitActor),
				               *GetNameSafe(HitComponent),
				               *DamageText);
			}
		}
	}
	else
	{
		SpawnProjectile(Direction, DamageSpecHandle);
	}
	return HitResult;
}

ABOProjectile* ABOFirearm::SpawnProjectile(const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpecHandle)
{
	if (!HasAuthority() || !ProjectileClass || !DamageSpecHandle.IsValid())
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector SafeDirection = Direction.GetSafeNormal();
	const FTransform SpawnTransform(SafeDirection.Rotation(), GetMuzzleTransform().GetLocation());

	ABOProjectile* Projectile = nullptr;
	if (UBlackoutPoolSubsystem* Pool = World->GetSubsystem<UBlackoutPoolSubsystem>())
	{
		Projectile = Cast<ABOProjectile>(Pool->SpawnFromPool(ProjectileClass, SpawnTransform));
	}

	if (!Projectile)
	{
		BO_LOG_POOL(Error, "SpawnProjectile failed: 풀에서 발사체를 가져오지 못함 (Weapon=%s, ProjectileClass=%s)",
		            *GetNameSafe(this),
		            *GetNameSafe(ProjectileClass.Get()));
		return nullptr;
	}

	Projectile->SetOwner(GetOwner());
	Projectile->SetInstigator(Cast<APawn>(GetOwner()));
	Projectile->InitFromSpec(DamageSpecHandle, GetSplashRadius());
	Projectile->Launch(SafeDirection);

	return Projectile;
}

FTransform ABOFirearm::GetMuzzleTransform() const
{
	if (WeaponMesh)
	{
		return WeaponMesh->GetSocketTransform(MuzzleSocket);
	}
	return GetActorTransform();
}

float ABOFirearm::GetFireRate() const
{
	return CachedFirearmStats.FireRate;
}

bool ABOFirearm::IsAutomatic() const
{
	return CachedFirearmStats.bIsAutomatic;
}

int32 ABOFirearm::GetMagazineSize() const
{
	return CachedFirearmStats.MagazineSize;
}

int32 ABOFirearm::GetMaxReserveAmmo() const
{
	return CachedFirearmStats.MaxReserveAmmo;
}

float ABOFirearm::GetSplashRadius() const
{
	return CachedFirearmStats.SplashRadius;
}
