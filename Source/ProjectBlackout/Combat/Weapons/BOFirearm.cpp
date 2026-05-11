#include "Combat/Weapons/BOFirearm.h"

#include "DrawDebugHelpers.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/AnimationAsset.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NiagaraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/Weapons/BOProjectile.h"
#include "Combat/Weapons/BOWeaponDebugUtils.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Core/BlackoutLog.h"
#include "Interfaces/BlackoutDamageable.h"
#include "Pool/BlackoutPoolSubsystem.h"

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
		ApplyFirearmStats(*FoundStats);
		return true;
	}

	return false;
}

void ABOFirearm::ApplyFirearmStats(const FBlackoutFirearmStat& FirearmStats)
{
	CachedFirearmStats = FirearmStats;
	ApplyCommonStats(CachedFirearmStats);
	bUseTwoHandedAnimation = CachedFirearmStats.bUseTwoHandedAnimation;
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

			World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, BlackoutCollisionChannels::WeaponTrace, QueryParams);
			AActor* HitActor = HitResult.GetActor();
			UPrimitiveComponent* HitComponent = HitResult.GetComponent();
			AActor* DamageTargetActor = BlackoutWeaponDebug::ResolveDamageTargetActor(HitResult);

			if (bDrawDebugHitscanRay)
			{
				const bool bHit = HitResult.bBlockingHit;
				const FVector DebugEnd = bHit ? HitResult.ImpactPoint : TraceEnd;
				const FColor DebugColor = BlackoutWeaponDebug::GetHitscanDebugColor(bHit, DamageTargetActor);
				DrawDebugLine(World, TraceStart, DebugEnd, DebugColor, false, DebugHitscanRayDuration, 0, DebugHitscanRayThickness);
			}

			float HealthBefore = 0.0f;
			bool bCapturedHealthBefore = bDrawDebugHitscanRay && BlackoutWeaponDebug::TryGetHealth(DamageTargetActor, HealthBefore);
			bool bAppliedDamage = false;

			if (HasAuthority() && HitResult.bBlockingHit && DamageSpecHandle.IsValid())
			{
				if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitComponent))
				{
					DamageTargetActor = HitboxComponent->GetOwner();
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
				const bool bCapturedHealthAfter = bAppliedDamage && BlackoutWeaponDebug::TryGetHealth(DamageTargetActor, HealthAfter);
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
				else if (!HasAuthority())
				{
					DamageText = TEXT("ClientOnly");
				}
				else if (!DamageSpecHandle.IsValid())
				{
					DamageText = TEXT("InvalidSpec");
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

bool ABOFirearm::UsesHitscan() const
{
	return bUseHitscan;
}

bool ABOFirearm::UsesTwoHandedAnimation() const
{
	return bUseTwoHandedAnimation;
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

TSubclassOf<ABOProjectile> ABOFirearm::GetProjectileClass() const
{
	return ProjectileClass;
}

float ABOFirearm::GetProjectileLaunchSpeed() const
{
	const ABOProjectile* ProjectileDefault = ProjectileClass ? ProjectileClass->GetDefaultObject<ABOProjectile>() : nullptr;
	return ProjectileDefault ? ProjectileDefault->GetInitialSpeed() : 0.0f;
}

float ABOFirearm::GetProjectileGravityScale() const
{
	const ABOProjectile* ProjectileDefault = ProjectileClass ? ProjectileClass->GetDefaultObject<ABOProjectile>() : nullptr;
	return ProjectileDefault ? ProjectileDefault->GetGravityScale() : 1.0f;
}

float ABOFirearm::GetProjectileCollisionRadius() const
{
	const ABOProjectile* ProjectileDefault = ProjectileClass ? ProjectileClass->GetDefaultObject<ABOProjectile>() : nullptr;
	return ProjectileDefault ? ProjectileDefault->GetCollisionRadius() : 0.0f;
}

float ABOFirearm::GetBaseSpreadDegrees() const { return CachedFirearmStats.BaseSpreadDegrees; }
float ABOFirearm::GetMaxSpreadDegrees() const { return CachedFirearmStats.MaxSpreadDegrees; }
float ABOFirearm::GetSpreadPerShot() const { return CachedFirearmStats.SpreadPerShot; }
float ABOFirearm::GetSpreadRecoveryRate() const { return CachedFirearmStats.SpreadRecoveryRate; }
float ABOFirearm::GetVerticalRecoilMin() const { return CachedFirearmStats.VerticalRecoilMin; }
float ABOFirearm::GetVerticalRecoilMax() const { return CachedFirearmStats.VerticalRecoilMax; }
float ABOFirearm::GetHorizontalRecoilRange() const { return CachedFirearmStats.HorizontalRecoilRange; }
float ABOFirearm::GetMaxRecoilPitchDegrees() const { return CachedFirearmStats.MaxRecoilPitchDegrees; }
float ABOFirearm::GetRecoilRecoveryFraction() const { return CachedFirearmStats.RecoilRecoveryFraction; }

bool ABOFirearm::PlayWeaponFireAnimation()
{
	if (!WeaponFireAnimation)
	{
		BO_LOG_CORE(Warning, "PlayWeaponFireAnimation failed: WeaponFireAnimation이 비어 있음 (Weapon=%s)", *GetName());
		return false;
	}

	if (!WeaponMesh)
	{
		BO_LOG_CORE(Warning, "PlayWeaponFireAnimation failed: WeaponMesh가 비어 있음 (Weapon=%s)", *GetName());
		return false;
	}

	if (WeaponMesh->GetAnimationMode() == EAnimationMode::AnimationSingleNode)
	{
		if (UAnimSingleNodeInstance* SingleNodeInstance = WeaponMesh->GetSingleNodeInstance())
		{
			if (SingleNodeInstance->GetCurrentAsset() == WeaponFireAnimation && SingleNodeInstance->IsPlaying())
			{
				return true;
			}
		}
	}

	WeaponMesh->PlayAnimation(WeaponFireAnimation, false);

	BO_LOG_CORE(Log,
		"PlayWeaponFireAnimation: Weapon=%s Local=%s Authority=%s Animation=%s",
		*GetName(),
		GetNetMode() != NM_DedicatedServer && GetOwner() && GetOwner()->GetLocalRole() == ROLE_AutonomousProxy ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(WeaponFireAnimation));

	return true;
}

void ABOFirearm::Multicast_PlayWeaponFireAnimation_Implementation()
{
	PlayWeaponFireAnimation();
}

bool ABOFirearm::StopWeaponFireAnimation()
{
	if (!WeaponMesh)
	{
		return false;
	}

	WeaponMesh->Stop();
	return true;
}

void ABOFirearm::Multicast_StopWeaponFireAnimation_Implementation()
{
	StopWeaponFireAnimation();
}

bool ABOFirearm::PlayWeaponReloadAnimation()
{
	if (!WeaponReloadAnimation)
	{
		BO_LOG_CORE(Warning, "PlayWeaponReloadAnimation failed: WeaponReloadAnimation이 비어 있음 (Weapon=%s)", *GetName());
		return false;
	}

	if (!WeaponMesh)
	{
		BO_LOG_CORE(Warning, "PlayWeaponReloadAnimation failed: WeaponMesh가 비어 있음 (Weapon=%s)", *GetName());
		return false;
	}

	if (WeaponMesh->GetAnimationMode() == EAnimationMode::AnimationSingleNode)
	{
		if (UAnimSingleNodeInstance* SingleNodeInstance = WeaponMesh->GetSingleNodeInstance())
		{
			if (SingleNodeInstance->GetCurrentAsset() == WeaponReloadAnimation && SingleNodeInstance->IsPlaying())
			{
				return true;
			}
		}
	}

	WeaponMesh->PlayAnimation(WeaponReloadAnimation, false);

	BO_LOG_CORE(Log,
		"PlayWeaponReloadAnimation: Weapon=%s Local=%s Authority=%s Animation=%s",
		*GetName(),
		GetNetMode() != NM_DedicatedServer && GetOwner() && GetOwner()->GetLocalRole() == ROLE_AutonomousProxy ? TEXT("true") : TEXT("false"),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(WeaponReloadAnimation));

	return true;
}

void ABOFirearm::Multicast_PlayWeaponReloadAnimation_Implementation()
{
	PlayWeaponReloadAnimation();
}

bool ABOFirearm::StopWeaponReloadAnimation()
{
	if (!WeaponMesh)
	{
		return false;
	}

	if (WeaponMesh->GetAnimationMode() == EAnimationMode::AnimationSingleNode)
	{
		if (UAnimSingleNodeInstance* SingleNodeInstance = WeaponMesh->GetSingleNodeInstance())
		{
			// 재장전 취소 시 탄창 분리 포즈가 남지 않도록 시작 프레임으로 되돌립니다.
			SingleNodeInstance->SetPosition(0.0f, false);
			SingleNodeInstance->SetPlaying(false);
			return true;
		}
	}

	WeaponMesh->Stop();
	return true;
}

void ABOFirearm::Multicast_StopWeaponReloadAnimation_Implementation()
{
	StopWeaponReloadAnimation();
}
