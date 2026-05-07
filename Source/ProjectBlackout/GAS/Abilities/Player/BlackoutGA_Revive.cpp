#include "GAS/Abilities/Player/BlackoutGA_Revive.h"

#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "EngineUtils.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "TimerManager.h"

UBlackoutGA_Revive::UBlackoutGA_Revive()
{
	InputID = EBlackoutAbilityInputID::Interact;
	bReplicateInputDirectly = true;

	ActivationOwnedTags.AddTag(BlackoutGameplayTags::State_Locked);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
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

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_Revive failed: CommitAbility 실패");
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
	}

	if (UCharacterMovementComponent* MovementComponent = CachedReviver->GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
	}
	
	

	ReviveElapsedTime = 0.0f;

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

	CachedReviver = nullptr;
	CachedTarget = nullptr;
	ReviveElapsedTime = 0.0f;

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

		const float RevivedHealth = FMath::Max(1.0f, MaxHealth * RevivedHealthPercent);
		CachedTarget->Server_ReviveFromDowned(RevivedHealth);

		BO_LOG_GAS(Log,
			"GA_Revive succeeded: Reviver=%s Target=%s RevivedHealth=%.1f",
			*GetNameSafe(CachedReviver.Get()),
			*GetNameSafe(CachedTarget.Get()),
			RevivedHealth);
	}

	K2_EndAbility();
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

	return FVector::DistSquared(Reviver->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(ReviveRange);
}
