#include "Combat/Weapons/BOProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/BlackoutWeaponCueLibrary.h"
#include "Components/SceneComponent.h"
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
	// 풀링 발사체는 수명이 짧고 재사용이 잦아 네트 도먼시 이득이 거의 없는 반면,
	// "도먼트 → 재사용 시 깨우기 → 같은 프레임 상태 변경 → 재도먼트" 사이클에서
	// 깨어난 직후 첫 업데이트의 NetState 델타가 누락되는 레이스를 유발한다.
	// 항상 Awake로 유지해 Launch의 ForceNetUpdate가 NetState를 확실히 복제하게 한다.
	SetNetDormancy(DORM_Awake);
	SetNetUpdateFrequency(60.f);
	SetMinNetUpdateFrequency(30.f);

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;
	Collision->OnComponentHit.AddDynamic(this, &ABOProjectile::OnHit);
	// 충돌 HitResult에서 표면 재질 기반 GCN을 고를 수 있게 피지컬 머티리얼을 반환합니다.
	Collision->bReturnMaterialOnMove = true;

	TrailCueAttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("TrailCueAttachComponent"));
	TrailCueAttachComponent->SetupAttachment(Collision);

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
	bReturnedToPool = false;
	
	// 풀 재사용 시 LifeSpan 타이머 재무장. InitialLifeSpan(BP 기본값)이 0이면 자동 정리 없음(히트로만 반환).
	SetLifeSpan(InitialLifeSpan);
	ApplyActiveState(true);
	Movement->Velocity = FVector::ZeroVector;
}

void ABOProjectile::BeginPlay()
{
	Super::BeginPlay();

	ApplyProjectileNetState();
}

void ABOProjectile::LifeSpanExpired()
{
	// 엔진 기본 동작(Destroy) 대신 풀로 반환. 권한 없는 클라이언트는 무시(서버 복제로 상태 동기화).
	SetLifeSpan(0.0f);
	if (HasAuthority())
	{
		ReturnToPool();
	}
}

void ABOProjectile::OnReturnToPool_Implementation()
{
	// 풀 대기 중 엔진 LifeSpan 타이머가 액터를 Destroy하지 않도록 취소.
	SetLifeSpan(0.0f);

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
		// 도먼시는 사용하지 않는다(생성자 주석 참고). 비활성 상태는 ForceNetUpdate로만 전파한다.
		ForceNetUpdate();
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
	IgnoreFirerWhenMoving();

	Movement->Velocity = Direction.GetSafeNormal() * Movement->InitialSpeed;
	Movement->SetActive(true, true);

	if (HasAuthority())
	{
		++ReplicatedNetState.StateId;
		ReplicatedNetState.bActive = true;
		ReplicatedNetState.Location = GetActorLocation();
		ReplicatedNetState.Direction = Direction.GetSafeNormal();
		ReplicatedNetState.Speed = Movement->InitialSpeed;
		ReplicatedNetState.GravityScale = Movement->ProjectileGravityScale;
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

	// 클라이언트도 발사자 자기충돌을 무시해야 스폰 즉시 발사자 콜리전에 막혀 정지하는 것을 방지.
	// Launch는 서버에서만 실행되므로 클라 발사 경로(이 함수)에서 별도로 설정한다.
	IgnoreFirerWhenMoving();

	SetActorLocationAndRotation(
		ReplicatedNetState.Location,
		ReplicatedNetState.Direction.Rotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);
	Movement->Velocity = FVector(ReplicatedNetState.Direction) * ReplicatedNetState.Speed;
	Movement->SetActive(true, true);
}

void ABOProjectile::IgnoreFirerWhenMoving()
{
	if (!Collision)
	{
		return;
	}

	// 풀 재사용 대비 이전 ignore 목록 클리어 후 현재 firer 등록.
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

	// Trail Gameplay Cue 로컬 제어
	if (TrailCueTag.IsValid())
	{
		if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
		{
			USceneComponent* AttachComponent = TrailCueAttachComponent ? TrailCueAttachComponent.Get() : GetRootComponent();
			if (AttachComponent)
			{
				AttachComponent->SetRelativeLocation(TrailCueLocationOffset);
			}

			FGameplayCueParameters Params;
			Params.EffectCauser = this;
			Params.TargetAttachComponent = AttachComponent;

			if (bIsActive)
			{
				CueManager->HandleGameplayCue(this, TrailCueTag, EGameplayCueEvent::OnActive, Params);
				CueManager->HandleGameplayCue(this, TrailCueTag, EGameplayCueEvent::WhileActive, Params);
			}
			else
			{
				CueManager->HandleGameplayCue(this, TrailCueTag, EGameplayCueEvent::Removed, Params);
			}
		}
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
	ExecuteProjectileGameplayCue(ImpactCueTag, CueParameters);
}

void ABOProjectile::ExecuteProjectileGameplayCue(FGameplayTag CueTag, const FGameplayCueParameters& CueParameters) const
{
	if (!CueTag.IsValid())
	{
		BO_LOG_CORE(Warning, "ExecuteProjectileGameplayCue skipped: CueTag가 유효하지 않음 (Projectile=%s)", *GetNameSafe(this));
		return;
	}

	if (ABlackoutPlayerCharacter* InstigatorCharacter = Cast<ABlackoutPlayerCharacter>(GetInstigator()))
	{
		InstigatorCharacter->Multicast_ExecuteWeaponGameplayCue(CueTag, CueParameters, false);
		return;
	}

	if (ABlackoutPlayerCharacter* OwnerCharacter = Cast<ABlackoutPlayerCharacter>(GetOwner()))
	{
		OwnerCharacter->Multicast_ExecuteWeaponGameplayCue(CueTag, CueParameters, false);
		return;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetCueAbilitySystemComponent())
	{
		AbilitySystemComponent->ExecuteGameplayCue(CueTag, CueParameters);
		return;
	}

	if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
	{
		CueManager->HandleGameplayCue(const_cast<ABOProjectile*>(this), CueTag, EGameplayCueEvent::Executed, CueParameters);
		return;
	}

	BO_LOG_CORE(Error, "ExecuteProjectileGameplayCue failed: ASC와 GameplayCueManager가 모두 유효하지 않음 (Projectile=%s, Cue=%s)",
	            *GetNameSafe(this),
	            *CueTag.ToString());
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

	// 같은 프레임에 OnHit이 여러 번 발생해도 풀 반환은 한 번만. (이중 반환 → 풀 중복 등록 방지)
	if (bReturnedToPool)
	{
		return;
	}
	bReturnedToPool = true;

	if (UWorld* World = GetWorld())
	{
		if (UBlackoutPoolSubsystem* Pool = World->GetSubsystem<UBlackoutPoolSubsystem>())
		{
			Pool->ReturnToPool(this);
		}
	}
}
