#include "Combat/Weapons/BOProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/BlackoutWeaponCueLibrary.h"
#include "Components/SphereComponent.h"
#include "Core/BlackoutLog.h"
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
	// 충돌 HitResult에서 표면 재질 기반 GCN을 고를 수 있게 피지컬 머티리얼을 반환합니다.
	Collision->bReturnMaterialOnMove = true;

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
	CueSet = FBlackoutWeaponCueSet();

	if (HasAuthority())
	{
		ForceNetUpdate();
		SetNetDormancy(DORM_DormantAll);
	}
}

void ABOProjectile::InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius)
{
	InitFromSpec(InDamageSpec, Radius, FBlackoutWeaponCueSet());
}

void ABOProjectile::InitFromSpec(const FGameplayEffectSpecHandle& InDamageSpec, float Radius, const FBlackoutWeaponCueSet& InCueSet)
{
	DamageSpec = InDamageSpec;
	SplashRadius = Radius;
	CueSet = InCueSet;
}

void ABOProjectile::Launch(const FVector& Direction)
{
	if (!Movement)
	{
		return;
	}

	// firer(발사자) 자기충돌 무시. 풀 재사용 대비 이전 ignore 목록 클리어 후 현재 firer 등록.
	if (Collision)
	{
		Collision->ClearMoveIgnoreActors();
		AActor* Firer = GetInstigator();
		if (!Firer)
		{
			Firer = GetOwner();
		}
		if (Firer)
		{
			Collision->IgnoreActorWhenMoving(Firer, true);
		}
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
	
	Movement->ProjectileGravityScale = ReplicatedNetState.GravityScale;

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
	// 발사자/소유자/자기 자신 충돌 무시 (firer PhysicsAsset 자기충돌로 스폰 즉시 소멸 방지)
	if (OtherActor == nullptr || OtherActor == this
		|| OtherActor == GetInstigator() || OtherActor == GetOwner())
	{
		return;
	}

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

	if (HasAuthority())
	{
		ExecuteImpactCue(Hit);
	}

	ReturnToPool();
}

void ABOProjectile::ExecuteImpactCue(const FHitResult& Hit) const
{
	if (!Hit.bBlockingHit)
	{
		return;
	}

	const FGameplayTag SurfaceTag = UBlackoutWeaponCueLibrary::ResolveSurfaceTag(Hit);
	const FGameplayTag ImpactCueTag = UBlackoutWeaponCueLibrary::ResolveImpactCueTag(CueSet, SurfaceTag);
	if (!ImpactCueTag.IsValid())
	{
		BO_LOG_CORE(Warning, "ExecuteImpactCue skipped: ImpactCueTag가 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	FGameplayCueParameters CueParameters = UBlackoutWeaponCueLibrary::BuildImpactCueParameters(const_cast<ABOProjectile*>(this), Hit);
	if (ABlackoutPlayerCharacter* InstigatorCharacter = Cast<ABlackoutPlayerCharacter>(GetInstigator()))
	{
		InstigatorCharacter->Multicast_ExecuteWeaponGameplayCue(ImpactCueTag, CueParameters, false);
		return;
	}

	if (ABlackoutPlayerCharacter* OwnerCharacter = Cast<ABlackoutPlayerCharacter>(GetOwner()))
	{
		OwnerCharacter->Multicast_ExecuteWeaponGameplayCue(ImpactCueTag, CueParameters, false);
		return;
	}

	UBlackoutWeaponCueLibrary::ExecuteWeaponCue(GetCueAbilitySystemComponent(), ImpactCueTag, CueParameters);
}

UAbilitySystemComponent* ABOProjectile::GetCueAbilitySystemComponent() const
{
	if (IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetInstigator()))
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface->GetAbilitySystemComponent())
		{
			return AbilitySystemComponent;
		}
	}

	if (IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner()))
	{
		return AbilitySystemInterface->GetAbilitySystemComponent();
	}

	return nullptr;
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
