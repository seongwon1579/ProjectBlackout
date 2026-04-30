#include "BlackoutCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLog.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/BlackoutCollisionChannels.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"

namespace
{
	bool HasInvulnerableState(const UAbilitySystemComponent* InAbilitySystemComponent)
	{
		if (!InAbilitySystemComponent)
		{
			return false;
		}

		FGameplayTagContainer InvulnerableTags;
		InvulnerableTags.AddTag(BlackoutGameplayTags::State_Invulnerable);
		
		return InAbilitySystemComponent->HasAnyMatchingGameplayTags(InvulnerableTags);
	}
}

ABlackoutCharacterBase::ABlackoutCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
}

void ABlackoutCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	GetCapsuleComponent()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
}

UAbilitySystemComponent* ABlackoutCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FGameplayTag ABlackoutCharacterBase::GetHitPartTag(FName BoneName) const
{
	return FGameplayTag();
}

bool ABlackoutCharacterBase::ApplyIncomingDamageSpec(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName)
{
	if (!HasAuthority() || !AbilitySystemComponent || !SpecHandle.IsValid())
	{
		return false;
	}

	// 이미 완전 사망한 대상은 추가 피해를 처리하지 않습니다.
	if (bIsDead)
	{
		return false;
	}

	if (HasInvulnerableState(AbilitySystemComponent))
	{
		BO_LOG_GAS(Verbose,
			"Damage ignored: Target=%s Bone=%s Reason=Invulnerable",
			*GetNameSafe(this),
			*BoneName.ToString());
		return false;
	}

	const float HealthBefore =
		AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	const float HealthAfter =
		AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());

	if (HealthBefore > 0.f && HealthAfter <= 0.f)
	{
		if (CanEnterDownedState())
		{
			OnDowned();
			return true;
		}

		OnDeath();
		return true;
	}

	if (HealthAfter < HealthBefore)
	{
		OnHitReact();
		return true;
	}

	return false;
}

void ABlackoutCharacterBase::ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName)
{
	ApplyIncomingDamageSpec(SpecHandle, BoneName);
}

void ABlackoutCharacterBase::OnDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	bIsDowned = false;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Downed);
	}

	BO_LOG_CORE(Log, "OnDeath: %s", *GetName());
}

void ABlackoutCharacterBase::OnDowned()
{
	if (bIsDead || bIsDowned)
	{
		return;
	}

	bIsDowned = true;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Downed);
	}

	BO_LOG_CORE(Log, "OnDowned: %s", *GetName());
}

bool ABlackoutCharacterBase::CanEnterDownedState() const
{
	return false;
}

void ABlackoutCharacterBase::OnHitReact()
{
}

void ABlackoutCharacterBase::OnStun()
{
}
