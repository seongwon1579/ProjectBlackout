#include "Combat/Weapons/BOMeridianGrenadeProjectile.h"

#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/Weapons/BOWeaponDebugUtils.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Core/BlackoutLog.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/StaticMesh.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GAS/Effects/ExecCalc_CombatReward.h"
#include "GameplayEffect.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutDamageable.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"

ABOMeridianGrenadeProjectile::ABOMeridianGrenadeProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Collision->InitSphereRadius(CollisionRadius);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetNotifyRigidBodyCollision(true);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(Collision);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultSphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (DefaultSphereMesh.Succeeded())
	{
		ProjectileMesh->SetStaticMesh(DefaultSphereMesh.Object);
	}

	Movement->InitialSpeed = 1800.0f;
	Movement->MaxSpeed = 2500.0f;

	ExplosionCueTag = BlackoutGameplayTags::GameplayCue_Weapon_Meridian_Explosion;
	ApplyProjectileSettingsToComponents();
}

void ABOMeridianGrenadeProjectile::OnSpawnFromPool_Implementation()
{
	Super::OnSpawnFromPool_Implementation();
	ClearAutoReturnTimer();
	ApplyProjectileSettingsToComponents();
	ResetGrenadeState();
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetActorTickEnabled(true);
}

void ABOMeridianGrenadeProjectile::OnReturnToPool_Implementation()
{
	ClearAutoReturnTimer();

	if (AActor* OwnerActor = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(OwnerActor, false);
	}
	if (APawn* InstigatorPawn = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(InstigatorPawn, false);
	}

	Super::OnReturnToPool_Implementation();
	ResetGrenadeState();
	ExplosionDamageSpec = FGameplayEffectSpecHandle();
	SetActorTickEnabled(false);
}

void ABOMeridianGrenadeProjectile::InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius)
{
	Super::InitFromSpec(InDamageSpec, Radius);
	ExplosionDamageSpec = InDamageSpec;
}

void ABOMeridianGrenadeProjectile::InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius, const FBlackoutWeaponCueSet& InCueSet)
{
	Super::InitFromSpec(InDamageSpec, Radius, InCueSet);
	ExplosionDamageSpec = InDamageSpec;
}

void ABOMeridianGrenadeProjectile::Launch(const FVector& Direction)
{
	ResetGrenadeState();
	PreviousLocation = GetActorLocation();
	ApplyProjectileSettingsToComponents();
	if (AActor* OwnerActor = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(OwnerActor, true);
	}
	if (APawn* InstigatorPawn = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(InstigatorPawn, true);
	}
	Super::Launch(Direction);
	StartAutoReturnTimer();
}

void ABOMeridianGrenadeProjectile::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyProjectileSettingsToComponents();
}

#if WITH_EDITOR
void ABOMeridianGrenadeProjectile::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyProjectileSettingsToComponents();
}
#endif

void ABOMeridianGrenadeProjectile::ApplyProjectileSettingsToComponents()
{
	if (Collision)
	{
		Collision->SetSphereRadius(FMath::Max(CollisionRadius, 0.0f), true);
	}

	if (ProjectileMesh)
	{
		ProjectileMesh->SetRelativeScale3D(FVector(FMath::Max(MeshScale, 0.0f)));
	}

	if (Movement)
	{
		Movement->bShouldBounce = true;
		Movement->Bounciness = Bounciness;
		Movement->Friction = Friction;
		Movement->ProjectileGravityScale = GravityScale;
		Movement->bBounceAngleAffectsFriction = true;
	}
}

void ABOMeridianGrenadeProjectile::SetProjectileMesh(UStaticMesh* InMesh)
{
	if (!ProjectileMesh)
	{
		BO_LOG_CORE(Error, "SetProjectileMesh failed: ProjectileMesh가 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	ProjectileMesh->SetStaticMesh(InMesh);
}

void ABOMeridianGrenadeProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateFuseState();
}

void ABOMeridianGrenadeProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UpdateFuseState();

	if (!HasAuthority())
	{
		return;
	}

	if (bExploded)
	{
		return;
	}

	if (bFuseArmed)
	{
		Explode(Hit);
		return;
	}

	ApplyImpactDamage(OtherActor, OtherComp, Hit);
	ExecuteImpactCue(Hit);
}

void ABOMeridianGrenadeProjectile::ResetGrenadeState()
{
	PreviousLocation = GetActorLocation();
	TraveledDistance = 0.0f;
	bFuseArmed = false;
	bExploded = false;
}

void ABOMeridianGrenadeProjectile::UpdateFuseState()
{
	if (bFuseArmed)
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	TraveledDistance += FVector::Dist(PreviousLocation, CurrentLocation);
	PreviousLocation = CurrentLocation;
	bFuseArmed = TraveledDistance >= ArmDistance;
}

FGameplayEffectSpecHandle ABOMeridianGrenadeProjectile::MakeImpactDamageSpec(const FGameplayEffectSpecHandle& SourceSpec) const
{
	if (!SourceSpec.IsValid() || !SourceSpec.Data.IsValid())
	{
		BO_LOG_CORE(Warning, "MakeImpactDamageSpec failed: DamageSpec이 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return FGameplayEffectSpecHandle();
	}

	FGameplayEffectSpecHandle NewSpecHandle(new FGameplayEffectSpec(*SourceSpec.Data.Get()));
	if (!NewSpecHandle.IsValid() || !NewSpecHandle.Data.IsValid())
	{
		BO_LOG_CORE(Error, "MakeImpactDamageSpec failed: 충격 DamageSpec 복제 실패 (Projectile=%s)", *GetNameSafe(this));
		return FGameplayEffectSpecHandle();
	}

	const float SourceDamage = SourceSpec.Data->GetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, false, 0.0f);
	NewSpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, SourceDamage * ImpactDamageMultiplier);
	return NewSpecHandle;
}

void ABOMeridianGrenadeProjectile::ApplyImpactDamage(AActor* OtherActor, UPrimitiveComponent* OtherComp, const FHitResult& Hit)
{
	const FGameplayEffectSpecHandle LocalImpactDamageSpec = MakeImpactDamageSpec(ExplosionDamageSpec);
	if (!LocalImpactDamageSpec.IsValid())
	{
		BO_LOG_CORE(Warning, "ApplyImpactDamage skipped: 충격 DamageSpec이 유효하지 않음 (Projectile=%s, Target=%s)",
		            *GetNameSafe(this),
		            *GetNameSafe(OtherActor));
		return;
	}

	if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(OtherComp))
	{
		HitboxComponent->ReceiveDamageSpec(LocalImpactDamageSpec);
		return;
	}

	if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(OtherActor))
	{
		Damageable->ReceiveDamageFromHitbox(LocalImpactDamageSpec, Hit.BoneName);
	}
}

void ABOMeridianGrenadeProjectile::Explode(const FHitResult& Hit)
{
	bExploded = true;
	ClearAutoReturnTimer();
	Movement->Deactivate();
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	const FVector ExplosionOrigin = Hit.bBlockingHit ? FVector(Hit.ImpactPoint) : GetActorLocation();
	ApplyExplosionDamage(ExplosionOrigin);
	if (bDrawDebugExplosionRadius && GetWorld())
	{
		DrawDebugSphere(
			GetWorld(),
			ExplosionOrigin,
			SplashRadius,
			DebugExplosionRadiusSegments,
			FColor::Orange,
			false,
			DebugExplosionRadiusDuration);
	}
	ExecuteExplosionCue(Hit);
	ReturnToPool();
}

void ABOMeridianGrenadeProjectile::ApplyExplosionDamage(const FVector& Origin)
{
	if (!ExplosionDamageSpec.IsValid())
	{
		BO_LOG_CORE(Error, "ApplyExplosionDamage failed: ExplosionDamageSpec이 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	if (SplashRadius <= 0.0f)
	{
		BO_LOG_CORE(Warning, "ApplyExplosionDamage skipped: 폭발 반경이 0 이하임 (Projectile=%s, Radius=%.1f)",
		            *GetNameSafe(this),
		            SplashRadius);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		BO_LOG_CORE(Error, "ApplyExplosionDamage failed: World가 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BOMeridianGrenadeExplosion), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	TArray<FOverlapResult> OverlapResults;
	const FCollisionShape ExplosionShape = FCollisionShape::MakeSphere(SplashRadius);
	const bool bHasOverlap = World->OverlapMultiByChannel(
		OverlapResults,
		Origin,
		FQuat::Identity,
		BlackoutCollisionChannels::WeaponTrace,
		ExplosionShape,
		QueryParams);

	if (!bHasOverlap)
	{
		return;
	}

	TSet<AActor*> DamagedActors;
	TArray<TWeakObjectPtr<AActor>> KilledTargets;
	for (const FOverlapResult& OverlapResult : OverlapResults)
	{
		AActor* HitActor = OverlapResult.GetActor();
		UPrimitiveComponent* HitComponent = OverlapResult.GetComponent();
		if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(HitComponent))
		{
			HitActor = HitboxComponent->GetOwner();
		}

		if (!IsValid(HitActor) || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor))
		{
			float HealthBeforeDamage = 0.0f;
			const bool bHadHealthBefore = BlackoutWeaponDebug::TryGetHealth(HitActor, HealthBeforeDamage);

			Damageable->ReceiveDamageFromHitbox(ExplosionDamageSpec, NAME_None);
			DamagedActors.Add(HitActor);

			float HealthAfterDamage = 0.0f;
			if (bHadHealthBefore
				&& HealthBeforeDamage > 0.0f
				&& BlackoutWeaponDebug::TryGetHealth(HitActor, HealthAfterDamage)
				&& HealthAfterDamage <= 0.0f)
			{
				KilledTargets.Add(HitActor);
			}
		}
	}

	if (KilledTargets.Num() >= 3)
	{
		for (const TWeakObjectPtr<AActor>& KilledTarget : KilledTargets)
		{
			AActor* TargetActor = KilledTarget.Get();
			IAbilitySystemInterface* TargetAbilityInterface = Cast<IAbilitySystemInterface>(TargetActor);
			UAbilitySystemComponent* TargetASC = TargetAbilityInterface ? TargetAbilityInterface->GetAbilitySystemComponent() : nullptr;
			FGameplayEffectSpecHandle RewardSpecHandle = BlackoutWeaponDebug::DuplicateGameplayEffectSpec(ExplosionDamageSpec);
			if (TargetASC && RewardSpecHandle.IsValid())
			{
				// 폭발 한 번으로 3마리 이상 처치가 확정된 뒤 데몰리션 보상 조건 태그를 후처리합니다.
				RewardSpecHandle.Data->AddDynamicAssetTag(BlackoutGameplayTags::Kill_MultiTarget_Count3);
				UExecCalc_CombatReward::ApplyConfiguredRewardEffect(RewardSpecHandle, TargetASC);
			}
		}
	}
}

void ABOMeridianGrenadeProjectile::ExecuteExplosionCue(const FHitResult& Hit)
{
	if (!ExplosionCueTag.IsValid())
	{
		BO_LOG_CORE(Warning, "ExecuteExplosionCue skipped: ExplosionCueTag가 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = Hit.bBlockingHit ? FVector(Hit.ImpactPoint) : GetActorLocation();
	CueParameters.Normal = Hit.ImpactNormal;
	CueParameters.Instigator = GetInstigator();
	CueParameters.EffectCauser = this;
	CueParameters.SourceObject = this;

	ExecuteProjectileGameplayCue(ExplosionCueTag, CueParameters);
}

void ABOMeridianGrenadeProjectile::StartAutoReturnTimer()
{
	ClearAutoReturnTimer();

	if (!HasAuthority() || AutoReturnDelay <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		BO_LOG_CORE(Error, "StartAutoReturnTimer failed: World가 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	World->GetTimerManager().SetTimer(
		AutoReturnTimerHandle,
		this,
		&ABOMeridianGrenadeProjectile::ReturnToPoolByLifetime,
		AutoReturnDelay,
		false);
}

void ABOMeridianGrenadeProjectile::ClearAutoReturnTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoReturnTimerHandle);
	}
}

void ABOMeridianGrenadeProjectile::ReturnToPoolByLifetime()
{
	if (!HasAuthority() || bExploded)
	{
		return;
	}

	bExploded = true;
	Movement->Deactivate();
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ReturnToPool();
}
