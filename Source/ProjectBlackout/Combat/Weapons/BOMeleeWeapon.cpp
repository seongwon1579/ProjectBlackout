#include "Combat/Weapons/BOMeleeWeapon.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Components/BoxComponent.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Core/BlackoutLog.h"
#include "Engine/World.h"
#include "GameplayEffect.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Interfaces/BlackoutDamageable.h"

ABOMeleeWeapon::ABOMeleeWeapon()
{
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetupAttachment(WeaponMesh);
	HitBox->SetCollisionObjectType(ECC_WorldDynamic);
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HitBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	HitBox->SetGenerateOverlapEvents(false);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 스윕(공격 판정) 중에만 활성화됨
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &ABOMeleeWeapon::HandleHitBoxBeginOverlap);
}

bool ABOMeleeWeapon::InitializeStatsFromDataTable()
{
	Super::InitializeStatsFromDataTable();

	if (const FBlackoutMeleeWeaponStat* FoundStats = MeleeStatsRow.GetRow<FBlackoutMeleeWeaponStat>(TEXT("BOMeleeWeapon::InitializeStatsFromDataTable")))
	{
		CachedMeleeStats = *FoundStats;
		ApplyCommonStats(CachedMeleeStats);
		SwingRadius = CachedMeleeStats.SwingRadius;
		return true;
	}

	return false;
}

TArray<FHitResult> ABOMeleeWeapon::PerformSweep(const FVector& Forward)
{
	TArray<FHitResult> HitResults;

	if (!HitBox || !GetWorld())
	{
		return HitResults;
	}

	const FVector SweepStart = HitBox->GetComponentLocation();
	const FVector SweepEnd = SweepStart + Forward.GetSafeNormal() * SwingRadius;
	const FQuat SweepRotation = HitBox->GetComponentQuat();
	const FCollisionShape CollisionShape = FCollisionShape::MakeBox(HitBox->GetScaledBoxExtent());

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BOMeleeWeapon_Sweep), false, GetOwner());
	QueryParams.AddIgnoredActor(this);

	GetWorld()->SweepMultiByChannel(HitResults, SweepStart, SweepEnd, SweepRotation, BlackoutCollisionChannels::WeaponTrace, CollisionShape, QueryParams);
	return HitResults;
}

void ABOMeleeWeapon::SetHitBoxActive(bool bActive)
{
	if (HitBox)
	{
		bHitBoxActive = bActive;
		HitBox->SetGenerateOverlapEvents(bActive);
		HitBox->SetCollisionEnabled(bActive ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void ABOMeleeWeapon::BeginHitWindow(TSubclassOf<UGameplayEffect> DamageEffectClass, float EffectLevel)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!DamageEffectClass)
	{
		BO_LOG_CORE(Warning, "BeginHitWindow failed: DamageEffectClass가 설정되지 않음 (Weapon=%s)", *GetNameSafe(this));
		EndHitWindow();
		return;
	}

	ActiveDamageEffectClass = DamageEffectClass;
	ActiveDamageEffectLevel = EffectLevel;
	HitActorsThisWindow.Reset();
	SetHitBoxActive(true);
}

void ABOMeleeWeapon::EndHitWindow()
{
	HitActorsThisWindow.Reset();
	ActiveDamageEffectClass = nullptr;
	ActiveDamageEffectLevel = 1.0f;
	SetHitBoxActive(false);
}

TArray<FHitResult> ABOMeleeWeapon::PerformSweepHit(const FVector& Forward, TSubclassOf<UGameplayEffect> DamageEffectClass, float EffectLevel)
{
	TArray<FHitResult> HitResults = PerformSweep(Forward);
	if (!HasAuthority())
	{
		return HitResults;
	}

	TSet<AActor*> DamagedActors;
	for (const FHitResult& HitResult : HitResults)
	{
		ApplyDamageToTarget(HitResult.GetActor(), HitResult.GetComponent(), HitResult.BoneName, DamageEffectClass, EffectLevel, DamagedActors);
	}

	return HitResults;
}

void ABOMeleeWeapon::HandleHitBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority() || !bHitBoxActive || !ActiveDamageEffectClass)
	{
		return;
	}

	ApplyDamageToTarget(OtherActor, OtherComp, SweepResult.BoneName, ActiveDamageEffectClass, ActiveDamageEffectLevel, HitActorsThisWindow);
}

FGameplayEffectSpecHandle ABOMeleeWeapon::BuildDamageSpec(TSubclassOf<UGameplayEffect> DamageEffectClass, float EffectLevel) const
{
	FGameplayEffectSpecHandle SpecHandle;
	if (!DamageEffectClass)
	{
		return SpecHandle;
	}

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOwner());
	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent)
	{
		BO_LOG_CORE(Warning, "BuildDamageSpec failed: AbilitySystemComponent가 없음 (Weapon=%s, Owner=%s)", *GetNameSafe(this), *GetNameSafe(GetOwner()));
		return SpecHandle;
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageEffectClass, EffectLevel, ContextHandle);
	if (!SpecHandle.IsValid())
	{
		BO_LOG_CORE(Warning, "BuildDamageSpec failed: GameplayEffectSpec 생성 실패 (Weapon=%s, EffectClass=%s)", *GetNameSafe(this), *GetNameSafe(DamageEffectClass));
		return SpecHandle;
	}

	const FGameplayTag DamageDataTag = GetDamageDataTag().IsValid()
		? GetDamageDataTag()
		: BlackoutGameplayTags::Data_Damage;

	SpecHandle.Data->SetSetByCallerMagnitude(DamageDataTag, GetBaseDamage());

	if (GetDamageTypeTag().IsValid())
	{
		SpecHandle.Data->DynamicAssetTags.AddTag(GetDamageTypeTag());
	}

	return SpecHandle;
}

bool ABOMeleeWeapon::ApplyDamageToTarget(
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FName BoneName,
	TSubclassOf<UGameplayEffect> DamageEffectClass,
	float EffectLevel,
	TSet<AActor*>& DamagedActors) const
{
	AActor* DamageTargetActor = OtherActor;
	if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(OtherComp))
	{
		DamageTargetActor = HitboxComponent->GetOwner();
	}

	if (!DamageTargetActor || DamageTargetActor == this || DamageTargetActor == GetOwner() || DamagedActors.Contains(DamageTargetActor))
	{
		return false;
	}

	if (!Cast<UBlackoutHitboxComponent>(OtherComp)
		&& DamageTargetActor->GetComponentByClass(UBlackoutHitboxComponent::StaticClass()) != nullptr)
	{
		return false;
	}

	const FGameplayEffectSpecHandle DamageSpecHandle = BuildDamageSpec(DamageEffectClass, EffectLevel);
	if (!DamageSpecHandle.IsValid())
	{
		return false;
	}

	if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(OtherComp))
	{
		HitboxComponent->ReceiveDamageSpec(DamageSpecHandle);
		DamagedActors.Add(DamageTargetActor);
		return true;
	}

	if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(DamageTargetActor))
	{
		Damageable->ReceiveDamageFromHitbox(DamageSpecHandle, BoneName);
		DamagedActors.Add(DamageTargetActor);
		return true;
	}

	return false;
}
