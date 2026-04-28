#include "BlackoutBossCharacter.h"
#include "Data/BOBossData.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameplayEffect.h"

ABlackoutBossCharacter::ABlackoutBossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentPhase = EBossPhase::None;
	PhaseIndex = 0;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

void ABlackoutBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && GetAbilitySystemComponent())
	{
		// 델리게이트 바인딩 (ASC 측 구현에 맞게 조정 필요)
		// GetAbilitySystemComponent()->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
		//     this, &ABlackoutBossCharacter::OnDamageReceived);
	}
}

void ABlackoutBossCharacter::OnReturnToPool_Implementation()
{
	// 보스는 풀링 대상이 아님. 풀 반환 대신 즉시 제거.
	Destroy();
}

void ABlackoutBossCharacter::OnDamageReceived(UAbilitySystemComponent* Source,
                                              const FGameplayEffectSpec& Spec,
                                              FActiveGameplayEffectHandle Handle)
{
	EvaluatePhaseTransition();
}

void ABlackoutBossCharacter::EvaluatePhaseTransition()
{
	if (!BossData || !HasAuthority())
	{
		return;
	}

	// TDD §6. 체력 기반 페이즈 전환 로직
	// float HealthRatio = CurrentHealth / MaxHealth;
	// if (BossData->PhaseHealthCutlines.IsValidIndex(PhaseIndex))
	// {
	//     if (HealthRatio <= BossData->PhaseHealthCutlines[PhaseIndex])
	//     {
	//         PhaseIndex++;
	//         // EBossPhase NextPhase = DetermineNextPhase(PhaseIndex);
	//         // OnPhaseChanged(NextPhase);
	//     }
	// }
}

void ABlackoutBossCharacter::OnPhaseChanged(EBossPhase NewPhase)
{
	CurrentPhase = NewPhase;
	BroadcastOnPhaseChanged();
}

void ABlackoutBossCharacter::BroadcastOnPhaseChanged()
{
	OnPhaseChangedDelegate.Broadcast(CurrentPhase);
}
