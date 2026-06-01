#include "GAS/Abilities/Player/BlackoutGA_Revive.h"

#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "EngineUtils.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "TimerManager.h"

namespace
{
	constexpr float ReviveRelicChargeCost = 1.0f;

	const UAbilitySystemComponent* ResolveAbilitySystemComponentFromActor(const AActor* AvatarActor)
	{
		if (!AvatarActor)
		{
			return nullptr;
		}

		if (const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(AvatarActor))
		{
			return AbilitySystemInterface->GetAbilitySystemComponent();
		}

		return nullptr;
	}
}

UBlackoutGA_Revive::UBlackoutGA_Revive()
{
	InputID = EBlackoutAbilityInputID::Interact;
	bReplicateInputDirectly = true;

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Locked);
	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Reviving);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);

	ReviveCueTag = BlackoutGameplayTags::GameplayCue_Character_Revive;
}

const UBlackoutGA_Revive* UBlackoutGA_Revive::GetActiveReviveAbilityFromActor(const AActor* AvatarActor)
{
	const UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponentFromActor(AvatarActor);
	if (!AbilitySystemComponent)
	{
		return nullptr;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		for (UGameplayAbility* AbilityInstance : AbilitySpec.GetAbilityInstances())
		{
			const UBlackoutGA_Revive* ReviveAbility = Cast<UBlackoutGA_Revive>(AbilityInstance);
			if (ReviveAbility && ReviveAbility->IsActive())
			{
				return ReviveAbility;
			}
		}
	}

	return nullptr;
}

float UBlackoutGA_Revive::GetReviveRangeForActor(const AActor* AvatarActor)
{
	const UAbilitySystemComponent* AbilitySystemComponent = ResolveAbilitySystemComponentFromActor(AvatarActor);
	const UBlackoutGA_Revive* FallbackAbility = GetDefault<UBlackoutGA_Revive>();
	float ResolvedReviveRange = FallbackAbility ? FallbackAbility->ReviveRange : 250.0f;

	if (!AbilitySystemComponent)
	{
		return ResolvedReviveRange;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (const UBlackoutGA_Revive* ReviveAbility = Cast<UBlackoutGA_Revive>(AbilitySpec.Ability))
		{
			ResolvedReviveRange = ReviveAbility->ReviveRange;
			break;
		}
	}

	return ResolvedReviveRange;
}

float UBlackoutGA_Revive::GetReviveProgressNormalized() const
{
	if (!IsActive())
	{
		return 0.0f;
	}

	if (ReviveDuration <= KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	if (const UWorld* World = GetWorld())
	{
		const float ElapsedTime = FMath::Max(0.0f, World->GetTimeSeconds() - LocalReviveStartTimeSeconds);
		return FMath::Clamp(ElapsedTime / ReviveDuration, 0.0f, 1.0f);
	}

	return 0.0f;
}

void UBlackoutGA_Revive::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_Revive activate requested");

	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		BO_LOG_GAS(Warning, "GA_Revive failed: ActorInfo 또는 AvatarActor가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedReviver = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
	if (!CachedReviver || CachedReviver->IsDead() || CachedReviver->IsDowned())
	{
		BO_LOG_GAS(Warning, "GA_Revive failed: Reviver가 유효하지 않거나 부활 불가 상태임");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CachedTarget = FindReviveTarget();
	if (!CanReviveTarget(CachedReviver.Get(), CachedTarget.Get()))
	{
		BO_LOG_GAS(Warning, "GA_Revive failed: 부활 가능한 대상이 없음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CachedReviver->HasAuthority() && !CachedTarget->TryBeginReviveInteraction(CachedReviver.Get(), ReviveDuration))
	{
		BO_LOG_GAS(Warning, "GA_Revive failed: 다른 플레이어가 이미 같은 대상을 부활 중임");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilitySystemComponent* ReviverAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!ReviverAbilitySystemComponent)
	{
		BO_LOG_GAS(Warning, "GA_Revive failed: ReviverAbilitySystemComponent가 비어 있음");
		if (CachedReviver->HasAuthority() && CachedTarget)
		{
			CachedTarget->EndReviveInteraction(CachedReviver.Get());
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 부활은 유물을 소모하는 상호작용이므로 시작 전에 최소 1개 보유 여부를 확인합니다.
	const float CurrentRelicCharges = ReviverAbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetRelicChargesAttribute());
	if (CurrentRelicCharges < ReviveRelicChargeCost)
	{
		BO_LOG_GAS(Warning,
			"GA_Revive failed: 유물 충전 수가 부족함 Reviver=%s Current=%.0f Need=%.0f",
			*GetNameSafe(CachedReviver.Get()),
			CurrentRelicCharges,
			ReviveRelicChargeCost);
		if (CachedReviver->HasAuthority() && CachedTarget)
		{
			CachedTarget->EndReviveInteraction(CachedReviver.Get());
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_Revive failed: CommitAbility 실패");
		if (CachedReviver->HasAuthority() && CachedTarget)
		{
			CachedTarget->EndReviveInteraction(CachedReviver.Get());
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (RevivePerformMontage)
	{
		// 오너 클라는 즉시 재생해서 입력 반응성을 확보
		if (CachedReviver->IsLocallyControlled())
		{
			CachedReviver->PlayRevivePerformMontage(RevivePerformMontage, 1.f);
		}

		// 서버는 다른 클라들에게 몽타주를 전파
		if (CachedReviver->HasAuthority())
		{
			CachedReviver->Multicast_PlayRevivePerformMontage(RevivePerformMontage, 1.f);
		}
	}

	if (UBlackoutCombatComponent* CombatComponent = CachedReviver->GetCombatComponent())
	{
		CombatComponent->StopFire();
		CombatComponent->HandlePrimaryActionReleased();
		CombatComponent->StopAim();
		CombatComponent->BeginEquippedWeaponHolsterOverride();
	}

	if (UCharacterMovementComponent* MovementComponent = CachedReviver->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}

	ReviveElapsedTime = 0.0f;
	LocalReviveStartTimeSeconds = CachedReviver->GetWorld() ? CachedReviver->GetWorld()->GetTimeSeconds() : 0.0f;

	if (CachedReviver->HasAuthority())
	{
		if (UWorld* World = CachedReviver->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				ReviveTickTimerHandle,
				this,
				&UBlackoutGA_Revive::HandleReviveTick,
				FMath::Max(0.01f, ReviveTickInterval),
				true,
				FMath::Max(0.01f, ReviveTickInterval));
		}
	}

	BO_LOG_GAS(Log,
		"GA_Revive started: Reviver=%s Target=%s Duration=%.2f",
		*GetNameSafe(CachedReviver.Get()),
		*GetNameSafe(CachedTarget.Get()),
		ReviveDuration);
}

void UBlackoutGA_Revive::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	if (IsActive())
	{
		BO_LOG_GAS(Log, "GA_Revive cancelled: input released");
		CancelRevive();
	}
}

void UBlackoutGA_Revive::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	BO_LOG_GAS(Log, "GA_Revive ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));

	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(ReviveTickTimerHandle);
		}
	}
	
	if (CachedReviver && RevivePerformMontage)
	{
		if (CachedReviver->IsLocallyControlled())
		{
			CachedReviver->StopRevivePerformMontage(RevivePerformMontage, 0.1f);
		}

		if (CachedReviver->HasAuthority())
		{
			CachedReviver->Multicast_StopRevivePerformMontage(RevivePerformMontage, 0.1f);
		}
	}

	if (CachedReviver)
	{
		if (UBlackoutCombatComponent* CombatComponent = CachedReviver->GetCombatComponent())
		{
			CombatComponent->EndEquippedWeaponHolsterOverride();
		}
	}

	if (CachedReviver && CachedReviver->HasAuthority() && CachedTarget)
	{
		CachedTarget->EndReviveInteraction(CachedReviver.Get());
	}

	CachedReviver = nullptr;
	CachedTarget = nullptr;
	ReviveElapsedTime = 0.0f;
	LocalReviveStartTimeSeconds = 0.0f;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Revive::HandleReviveTick()
{
	if (!IsActive())
	{
		return;
	}

	if (!CachedReviver || !CachedReviver->HasAuthority())
	{
		return;
	}

	if (!CanReviveTarget(CachedReviver.Get(), CachedTarget.Get()))
	{
		BO_LOG_GAS(Log, "GA_Revive cancelled: 대상 또는 시전자 상태가 더 이상 유효하지 않음");
		CancelRevive();
		return;
	}

	if (CachedReviver && CachedReviver->IsHitReactMontagePlaying())
	{
		BO_LOG_GAS(Log, "GA_Revive cancelled: 시전자가 피격 리액션 상태에 들어감");
		CancelRevive();
		return;
	}

	ReviveElapsedTime += FMath::Max(0.01f, ReviveTickInterval);
	if (ReviveElapsedTime + KINDA_SMALL_NUMBER < ReviveDuration)
	{
		return;
	}

	FinishRevive();
}

void UBlackoutGA_Revive::FinishRevive()
{
	if (!IsActive())
	{
		return;
	}

	if (!CachedReviver || !CachedReviver->HasAuthority())
	{
		return;
	}

	if (!CanReviveTarget(CachedReviver.Get(), CachedTarget.Get()))
	{
		BO_LOG_GAS(Log, "GA_Revive cancelled: 완료 시점에 대상 유효성 검사를 통과하지 못함");
		CancelRevive();
		return;
	}

	if (CachedReviver && CachedReviver->HasAuthority())
	{
		UAbilitySystemComponent* ReviverAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
		if (!ReviverAbilitySystemComponent)
		{
			BO_LOG_GAS(Warning, "GA_Revive failed: ReviverAbilitySystemComponent가 비어 있음");
			CancelRevive();
			return;
		}

		UAbilitySystemComponent* TargetAbilitySystemComponent = CachedTarget->GetAbilitySystemComponent();
		if (!TargetAbilitySystemComponent)
		{
			BO_LOG_GAS(Warning, "GA_Revive failed: TargetAbilitySystemComponent가 비어 있음");
			CancelRevive();
			return;
		}

		const float MaxHealth = TargetAbilitySystemComponent->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
		if (MaxHealth <= 0.0f)
		{
			BO_LOG_GAS(Warning, "GA_Revive failed: Target MaxHealth가 0 이하임");
			CancelRevive();
			return;
		}

		// 입력 유지 중 다른 사용처로 유물이 빠질 수 있으므로 완료 직전에 다시 한 번 확인합니다.
		const float CurrentRelicCharges = ReviverAbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetRelicChargesAttribute());
		if (CurrentRelicCharges < ReviveRelicChargeCost)
		{
			BO_LOG_GAS(Warning,
				"GA_Revive failed: 완료 시점에 유물 충전 수가 부족함 Reviver=%s Current=%.0f Need=%.0f",
				*GetNameSafe(CachedReviver.Get()),
				CurrentRelicCharges,
				ReviveRelicChargeCost);
			CancelRevive();
			return;
		}

		const float RevivedHealth = FMath::Max(1.0f, MaxHealth * RevivedHealthPercent);
		ReviverAbilitySystemComponent->ApplyModToAttribute(
			UBlackoutPlayerAttributeSet::GetRelicChargesAttribute(),
			EGameplayModOp::Additive,
			-ReviveRelicChargeCost);
		CachedTarget->Server_ReviveFromDowned(RevivedHealth);
		ExecuteReviveCue(TargetAbilitySystemComponent, RevivedHealth);

		BO_LOG_GAS(Log,
			"GA_Revive succeeded: Reviver=%s Target=%s RevivedHealth=%.1f RemainingRelics=%.0f",
			*GetNameSafe(CachedReviver.Get()),
			*GetNameSafe(CachedTarget.Get()),
			RevivedHealth,
			ReviverAbilitySystemComponent->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetRelicChargesAttribute()));
	}

	K2_EndAbility();
}

void UBlackoutGA_Revive::ExecuteReviveCue(UAbilitySystemComponent* TargetAbilitySystemComponent, float RevivedHealth) const
{
	if (!ReviveCueTag.IsValid())
	{
		return;
	}

	if (!CachedReviver || !CachedTarget || !CachedReviver->HasAuthority())
	{
		return;
	}

	USkeletalMeshComponent* TargetMesh = CachedTarget->GetMesh();
	if (!TargetAbilitySystemComponent || !TargetMesh)
	{
		BO_LOG_GAS(Warning, "부활 GCN 실행 실패: Target ASC 또는 Mesh가 유효하지 않습니다. ASC=%s Target=%s Mesh=%s",
			*GetNameSafe(TargetAbilitySystemComponent),
			*GetNameSafe(CachedTarget.Get()),
			*GetNameSafe(TargetMesh));
		return;
	}

	FGameplayCueParameters CueParameters;
	CueParameters.Location = CachedTarget->GetActorLocation();
	CueParameters.Normal = CachedTarget->GetActorForwardVector();
	CueParameters.RawMagnitude = RevivedHealth;
	CueParameters.Instigator = CachedReviver->GetInstigator();
	CueParameters.EffectCauser = CachedReviver.Get();
	CueParameters.SourceObject = CachedTarget.Get();
	CueParameters.TargetAttachComponent = TargetMesh;

	TargetAbilitySystemComponent->ExecuteGameplayCue(ReviveCueTag, CueParameters);
}

void UBlackoutGA_Revive::CancelRevive()
{
	if (!IsActive())
	{
		return;
	}

	K2_CancelAbility();
}

ABlackoutPlayerCharacter* UBlackoutGA_Revive::FindReviveTarget() const
{
	const ABlackoutPlayerCharacter* Reviver = CachedReviver.Get();
	if (!Reviver || !Reviver->GetWorld())
	{
		return nullptr;
	}

	const float MaxRangeSquared = FMath::Square(ReviveRange);
	ABlackoutPlayerCharacter* BestTarget = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (TActorIterator<ABlackoutPlayerCharacter> It(Reviver->GetWorld()); It; ++It)
	{
		ABlackoutPlayerCharacter* Candidate = *It;
		if (!CanReviveTarget(Reviver, Candidate))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(Reviver->GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSquared > MaxRangeSquared || DistanceSquared >= BestDistanceSquared)
		{
			continue;
		}

		BestDistanceSquared = DistanceSquared;
		BestTarget = Candidate;
	}

	return BestTarget;
}

bool UBlackoutGA_Revive::CanReviveTarget(const ABlackoutPlayerCharacter* Reviver, const ABlackoutPlayerCharacter* Target) const
{
	if (!Reviver || !Target)
	{
		return false;
	}

	if (Reviver == Target)
	{
		return false;
	}

	if (Reviver->IsDead() || Reviver->IsDowned())
	{
		return false;
	}

	if (Target->IsDead() || !Target->IsDowned())
	{
		return false;
	}

	if (Target->IsBeingRevived() && Target != CachedTarget.Get())
	{
		return false;
	}

	return FVector::DistSquared(Reviver->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(ReviveRange);
}
