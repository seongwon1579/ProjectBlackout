#include "BlackoutCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLog.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Framework/BlackoutPlayerController.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

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

	ABlackoutPlayerController* ResolveBlackoutPlayerControllerFromActor(AActor* SourceActor)
	{
		AActor* CurrentActor = SourceActor;
		for (int32 Depth = 0; CurrentActor && Depth < 4; ++Depth)
		{
			if (ABlackoutPlayerController* BlackoutPlayerController = Cast<ABlackoutPlayerController>(CurrentActor))
			{
				return BlackoutPlayerController;
			}

			if (AController* SourceController = Cast<AController>(CurrentActor))
			{
				if (ABlackoutPlayerController* BlackoutPlayerController = Cast<ABlackoutPlayerController>(SourceController))
				{
					return BlackoutPlayerController;
				}
			}

			if (APawn* SourcePawn = Cast<APawn>(CurrentActor))
			{
				if (ABlackoutPlayerController* BlackoutPlayerController =
					Cast<ABlackoutPlayerController>(SourcePawn->GetController()))
				{
					return BlackoutPlayerController;
				}
			}

			CurrentActor = CurrentActor->GetOwner();
		}

		return nullptr;
	}

	ABlackoutPlayerController* ResolveDamageNumberOwner(const FGameplayEffectSpecHandle& SpecHandle)
	{
		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
		{
			return nullptr;
		}

		const FGameplayEffectContextHandle& EffectContext = SpecHandle.Data->GetContext();
		TArray<AActor*, TInlineAllocator<3>> SourceCandidates;
		SourceCandidates.Add(EffectContext.GetEffectCauser());
		SourceCandidates.Add(EffectContext.GetInstigator());
		SourceCandidates.Add(EffectContext.GetOriginalInstigator());

		for (AActor* SourceCandidate : SourceCandidates)
		{
			if (ABlackoutPlayerController* BlackoutPlayerController =
				ResolveBlackoutPlayerControllerFromActor(SourceCandidate))
			{
				return BlackoutPlayerController;
			}
		}

		return nullptr;
	}

	FVector ResolveDamageNumberWorldLocation(const ABlackoutCharacterBase* TargetCharacter, FName BoneName)
	{
		if (!TargetCharacter)
		{
			return FVector::ZeroVector;
		}

		if (const USkeletalMeshComponent* CharacterMesh = TargetCharacter->GetMesh())
		{
			if (BoneName != NAME_None && CharacterMesh->GetBoneIndex(BoneName) != INDEX_NONE)
			{
				return CharacterMesh->GetBoneLocation(BoneName) + FVector(0.f, 0.f, 12.f);
			}
		}

		const UCapsuleComponent* CapsuleComponent = TargetCharacter->GetCapsuleComponent();
		const float HeightOffset = CapsuleComponent ? CapsuleComponent->GetScaledCapsuleHalfHeight() : 50.f;
		return TargetCharacter->GetActorLocation() + FVector(0.f, 0.f, HeightOffset);
	}

	bool IsCriticalDamageSpec(const FGameplayEffectSpecHandle& SpecHandle, const FGameplayTag& HitPartTag)
	{
		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
		{
			return false;
		}

		if (SpecHandle.Data->GetSetByCallerMagnitude(BlackoutGameplayTags::Body_WeakSpot, false, 0.f) > 0.f)
		{
			return true;
		}

		return HitPartTag.MatchesTagExact(BlackoutGameplayTags::Body_WeakSpot);
	}

	bool ShouldSuppressAuthoritativeDamageNumber(const FGameplayEffectSpecHandle& SpecHandle)
	{
		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
		{
			return false;
		}

		return SpecHandle.Data->GetSetByCallerMagnitude(
			BlackoutGameplayTags::Data_DamageNumber_PredictedOnly,
			false,
			0.0f) > 0.0f;
	}
}

ABlackoutCharacterBase::ABlackoutCharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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

void ABlackoutCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutCharacterBase, bIsDowned);
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

	const float AppliedDamage = FMath::Max(0.f, HealthBefore - HealthAfter);
	if (AppliedDamage > 0.f && !ShouldSuppressAuthoritativeDamageNumber(SpecHandle))
	{
		if (ABlackoutPlayerController* SourcePlayerController = ResolveDamageNumberOwner(SpecHandle))
		{
			const bool bIsCritical = IsCriticalDamageSpec(SpecHandle, GetHitPartTag(BoneName));
			const FVector DamageNumberWorldLocation = ResolveDamageNumberWorldLocation(this, BoneName);

			// 실제 적용된 데미지만 서버에서 계산해 사격한 클라 HUD로 전달합니다.
			SourcePlayerController->Client_ShowDamageNumberAtLocation(
				AppliedDamage,
				DamageNumberWorldLocation,
				bIsCritical);
		}
	}

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
		AbilitySystemComponent->CancelHealthRegenOverTime();
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
		AbilitySystemComponent->CancelHealthRegenOverTime();
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

void ABlackoutCharacterBase::OnRep_DownedState()
{
	BO_LOG_CORE(Log,
		"OnRep_DownedState: %s Downed=%s",
		*GetNameSafe(this),
		bIsDowned ? TEXT("true") : TEXT("false"));

	HandleDownedStateChanged();
}

void ABlackoutCharacterBase::HandleDownedStateChanged()
{
}
