#include "BlackoutBossCharacter.h"
#include "AI/BOAggroComponent.h"
#include "Data/BOBossData.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameplayEffect.h"

ABlackoutBossCharacter::ABlackoutBossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentPhase = EBossPhase::None;
	PhaseIndex = 0;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	AggroComponent = CreateDefaultSubobject<UBOAggroComponent>(TEXT("AggroComponent"));
}

void ABlackoutBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
			this, &ABlackoutBossCharacter::OnDamageReceived);
	}
}

void ABlackoutBossCharacter::OnReturnToPool_Implementation()
{
	Destroy();
}

void ABlackoutBossCharacter::OnDamageReceived(UAbilitySystemComponent* Source,
                                              const FGameplayEffectSpec& Spec,
                                              FActiveGameplayEffectHandle Handle)
{
	EvaluatePhaseTransition();

	if (AggroComponent)
	{
		AActor* SourceActor = Spec.GetContext().GetInstigator();
		APawn* InstigatorPawn = Cast<APawn>(SourceActor);
		if (!InstigatorPawn)
		{
			if (AController* SourceController = Cast<AController>(SourceActor))
			{
				InstigatorPawn = SourceController->GetPawn();
			}
		}

		if (InstigatorPawn)
		{
			AggroComponent->AddThreat(InstigatorPawn, 1.f);
		}
	}
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
	//         EBossPhase NextPhase = DetermineNextPhase(PhaseIndex);
	//         OnPhaseChanged(NextPhase);
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
