#include "BlackoutBossCharacter.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "AI/BOAggroComponent.h"
#include "Data/BOBossData.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Kismet/GameplayStatics.h"
#include "UI/BlackoutHUD.h"
#include "UI/BlackoutEnemyHUDWidgetController.h"

ABlackoutBossCharacter::ABlackoutBossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentPhase = EBossPhase::None;
	PhaseIndex = 0;

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	AggroComponent = CreateDefaultSubobject<UBOAggroComponent>(TEXT("AggroComponent"));
}

void ABlackoutBossCharacter::OnDeath()
{
	Super::OnDeath();

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic("Dead");
		}
	}
}

void ABlackoutBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
			this, &ABlackoutBossCharacter::OnDamageReceived);
	}
	
	GetWorld()->GetTimerManager().SetTimerForNextTick(
	this, &ABlackoutBossCharacter::TryBindToHUD);
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
	// if (!BossData || !HasAuthority())
	// {
	// 	return;
	// }

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

void ABlackoutBossCharacter::TryBindToHUD()
{
	bool bBound = false;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue;

		if (ABlackoutHUD* BlackoutHUD = PC->GetHUD<ABlackoutHUD>())
		{
			if (UBlackoutEnemyHUDWidgetController* EnemyHUDController = BlackoutHUD->GetEnemyHUDWidgetController())
			{
				EnemyHUDController->BindToEnemy(GetAbilitySystemComponent(), FText::FromString(TEXT("Ravager")));
				bBound = true;
			}
		}
	}

	// HUD가 아직 준비되지 않은 경우 다음 틱에 재시도한다.
	if (!bBound)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ABlackoutBossCharacter::TryBindToHUD);
	}
}
