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
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "GAS/Effects/ExecCalc_CombatReward.h"
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

	const AActor* ResolveDamageSourceActor(const FGameplayEffectSpecHandle& SpecHandle)
	{
		if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
		{
			return nullptr;
		}

		const FGameplayEffectContextHandle& EffectContext = SpecHandle.Data->GetContext();
		TArray<const AActor*, TInlineAllocator<4>> SourceCandidates;
		SourceCandidates.Add(EffectContext.GetEffectCauser());
		SourceCandidates.Add(EffectContext.GetInstigator());
		SourceCandidates.Add(EffectContext.GetOriginalInstigator());

		if (const UObject* SourceObject = EffectContext.GetSourceObject())
		{
			if (const AActor* SourceActor = Cast<AActor>(SourceObject))
			{
				SourceCandidates.Add(SourceActor);
			}
		}

		for (const AActor* SourceCandidate : SourceCandidates)
		{
			if (IsValid(SourceCandidate))
			{
				return SourceCandidate;
			}
		}

		return nullptr;
	}

	FVector ResolveDamageSourceWorldLocation(const FGameplayEffectSpecHandle& SpecHandle, const AActor* FallbackTargetActor)
	{
		if (const AActor* DamageSourceActor = ResolveDamageSourceActor(SpecHandle))
		{
			return DamageSourceActor->GetActorLocation();
		}

		return FallbackTargetActor ? FallbackTargetActor->GetActorLocation() : FVector::ZeroVector;
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

	// 캡슐 콜리전은 무기 판정용으로 쓰지 않는다.
	GetCapsuleComponent()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Ignore);
	
	// 메시 내의 Physics Asset을 무기 판정용으로 사용한다.
	GetMesh()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);
}

void ABlackoutCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 혹시 모르니 BeginPlay에서 한번 더 덮어씌운다.
	GetCapsuleComponent()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(BlackoutCollisionChannels::WeaponTrace, ECR_Block);

	BindDownedStateTagEvent();
}

void ABlackoutCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutCharacterBase, bIsDead);
	DOREPLIFETIME(ABlackoutCharacterBase, bReplicatedDownedStateTag);
}

UAbilitySystemComponent* ABlackoutCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool ABlackoutCharacterBase::IsDowned() const
{
	return AbilitySystemComponent
		? AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed)
		: bCachedIsDowned;
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
	const float StunBefore =
		AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStunGaugeAttribute());
	const FGameplayTag HitPartTag = GetHitPartTag(BoneName);

	// 약점 치명타 처치 보상은 실제 사망 확정 직후 같은 Spec으로 판정되므로, 데미지 적용 전에 태그를 보강합니다.
	if (IsCriticalDamageSpec(SpecHandle, HitPartTag))
	{
		SpecHandle.Data->AddDynamicAssetTag(BlackoutGameplayTags::Kill_WeakSpot);
	}

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

	const float HealthAfter =
		AbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float StunAfter =
		AbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetStunGaugeAttribute());

	const float AppliedDamage = FMath::Max(0.f, HealthBefore - HealthAfter);
	if (AppliedDamage > 0.f && !ShouldSuppressAuthoritativeDamageNumber(SpecHandle))
	{
		if (ABlackoutPlayerController* SourcePlayerController = ResolveDamageNumberOwner(SpecHandle))
		{
			const bool bIsCritical = IsCriticalDamageSpec(SpecHandle, HitPartTag);
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

		// 치명 피해 확정 직후, State.Dead 부여와 풀 반환 사이드 이펙트가 실행되기 전에 보상 GE를 먼저 적용합니다.
		UExecCalc_CombatReward::ApplyConfiguredRewardEffect(SpecHandle, AbilitySystemComponent);
		OnDeath();
		return true;
	}

	if (HealthAfter < HealthBefore || StunAfter > StunBefore)
	{
		const FVector DamageSourceLocation = ResolveDamageSourceWorldLocation(SpecHandle, this);
		HandlePostDamageReaction(AppliedDamage, StunBefore, StunAfter, DamageSourceLocation);
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

	const bool bWasDowned = IsDowned();
	SetDeadStateActive(true);
	SetDownedStateActive(false);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelHealthRegenOverTime();
	}

	if (bWasDowned)
	{
		RefreshDownedPresentationCache();
	}

	BO_LOG_CORE(Log, "OnDeath: %s", *GetName());
}

void ABlackoutCharacterBase::OnDowned()
{
	if (bIsDead || IsDowned())
	{
		return;
	}

	SetDownedStateActive(true);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelHealthRegenOverTime();
	}

	BO_LOG_CORE(Log, "OnDowned: %s", *GetName());
}

bool ABlackoutCharacterBase::CanEnterDownedState() const
{
	return false;
}

void ABlackoutCharacterBase::OnHitReact(float AppliedDamage, const FVector& DamageSourceLocation)
{
	(void)AppliedDamage;
	(void)DamageSourceLocation;
}

void ABlackoutCharacterBase::HandlePostDamageReaction(
	float AppliedDamage,
	float StunBefore,
	float StunAfter,
	const FVector& DamageSourceLocation)
{
	(void)StunBefore;
	(void)StunAfter;

	if (AppliedDamage > 0.0f)
	{
		OnHitReact(AppliedDamage, DamageSourceLocation);
	}
}

void ABlackoutCharacterBase::OnStun()
{
}

void ABlackoutCharacterBase::HandleDownedStateChanged(bool bWasDowned, bool bIsDowned)
{
}

void ABlackoutCharacterBase::BindDownedStateTagEvent()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	if (BoundDownedTagAbilitySystemComponent.Get() == AbilitySystemComponent && DownedTagChangedHandle.IsValid())
	{
		ApplyReplicatedDownedStateTag();
		ApplyReplicatedDeadStateTag();
		RefreshDownedPresentationCache();
		return;
	}

	if (UAbilitySystemComponent* PreviousAbilitySystemComponent = BoundDownedTagAbilitySystemComponent.Get())
	{
		if (DownedTagChangedHandle.IsValid())
		{
			PreviousAbilitySystemComponent
				->RegisterGameplayTagEvent(BlackoutGameplayTags::State_Downed, EGameplayTagEventType::NewOrRemoved)
				.Remove(DownedTagChangedHandle);
		}
	}

	DownedTagChangedHandle = AbilitySystemComponent
		->RegisterGameplayTagEvent(BlackoutGameplayTags::State_Downed, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ABlackoutCharacterBase::HandleDownedTagChanged);
	BoundDownedTagAbilitySystemComponent = AbilitySystemComponent;

	ApplyReplicatedDownedStateTag();
	ApplyReplicatedDeadStateTag();
	RefreshDownedPresentationCache();
}

void ABlackoutCharacterBase::SetDownedStateActive(bool bNewDowned)
{
	if (HasAuthority())
	{
		bReplicatedDownedStateTag = bNewDowned;
	}

	if (!AbilitySystemComponent)
	{
		SetDownedPresentationCache(bNewDowned);
		return;
	}

	if (bNewDowned)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Downed);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Downed);
	}

	RefreshDownedPresentationCache();
}

void ABlackoutCharacterBase::SetDeadStateActive(bool bNewDead)
{
	if (HasAuthority())
	{
		bIsDead = bNewDead;

		// 서버 측에서도 즉시 캡슐 충돌 설정을 변경하여 플레이어 및 에네미 사망 시 동기화를 일치시킵니다.
		if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
		{
			CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, bNewDead ? ECR_Ignore : ECR_Block);
			CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, bNewDead ? ECR_Ignore : ECR_Block);
		}
	}

	if (!AbilitySystemComponent)
	{
		return;
	}

	if (bNewDead)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Dead))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Dead);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Dead);
	}
}

void ABlackoutCharacterBase::ApplyReplicatedDownedStateTag()
{
	if (HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	if (bReplicatedDownedStateTag)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Downed);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Downed);
	}
}

void ABlackoutCharacterBase::ApplyReplicatedDeadStateTag()
{
	if (HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	if (bIsDead)
	{
		if (!AbilitySystemComponent->HasMatchingGameplayTag(BlackoutGameplayTags::State_Dead))
		{
			AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Dead);
		}
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Dead);
	}
}

void ABlackoutCharacterBase::RefreshDownedPresentationCache()
{
	SetDownedPresentationCache(IsDowned());
}

void ABlackoutCharacterBase::HandleDownedTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	SetDownedPresentationCache(NewCount > 0);
}

void ABlackoutCharacterBase::SetDownedPresentationCache(bool bNewDowned)
{
	if (bCachedIsDowned == bNewDowned)
	{
		return;
	}

	const bool bWasDowned = bCachedIsDowned;
	bCachedIsDowned = bNewDowned;

	BO_LOG_CORE(Log,
		"Downed state changed: %s Downed=%s",
		*GetNameSafe(this),
		bCachedIsDowned ? TEXT("true") : TEXT("false"));

	BroadcastDownedStateChanged();
	HandleDownedStateChanged(bWasDowned, bCachedIsDowned);
}

void ABlackoutCharacterBase::OnRep_DownedStateTagBridge()
{
	if (!AbilitySystemComponent)
	{
		SetDownedPresentationCache(bReplicatedDownedStateTag);
		return;
	}

	ApplyReplicatedDownedStateTag();
	RefreshDownedPresentationCache();
}

void ABlackoutCharacterBase::OnRep_DeadStateTagBridge()
{
	ApplyReplicatedDeadStateTag();

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		if (bIsDead)
		{
			// 클라이언트 측 사망 복제 수신 시 폰 채널과의 충돌 반응을 무시로 전환하여 플레이어 캐릭터들이 통과할 수 있게 합니다.
			CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		}
		else
		{
			// 부활 복제 수신 시 캡슐의 Pawn 충돌 반응을 다시 블록으로 원상복구합니다.
			CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
			CapsuleComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
		}
	}
}

void ABlackoutCharacterBase::BroadcastDownedStateChanged()
{
	OnDownedStateChanged.Broadcast(bCachedIsDowned);
	OnDownedStateChangedNative.Broadcast(this, bCachedIsDowned);
}
