#include "BlackoutBossCharacter.h"
#include "AIController.h"
#include "BlackoutBaseAttributeSet.h"
#include "BlackoutBossAIController.h"
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

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	for (const auto& [_, Data] : BossAbilityData)
	{
		if (!Data || !Data->GrantedAbility) continue;

		ASC->GiveAbility(FGameplayAbilitySpec(Data->GrantedAbility, 1));
	}

	if (HasAuthority())
	{
		// ASC()->OnGameplayEffectAppliedDelegateToSelf.AddUObject(
		// 	this, &ABlackoutBossCharacter::OnDamageReceived);

		ASC->GetGameplayAttributeValueChangeDelegate(
			   UBlackoutBaseAttributeSet::GetHealthAttribute())
		   .AddUObject(this, &ABlackoutBossCharacter::OnDamageReceived);
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		this, &ABlackoutBossCharacter::TryBindToHUD);
}

void ABlackoutBossCharacter::OnReturnToPool_Implementation()
{
	Destroy();
}

UBORavagerData* ABlackoutBossCharacter::GetPatternData(FGameplayTag AbilityTag) const
{
	const TObjectPtr<UBORavagerData>* Found = BossAbilityData.Find(AbilityTag);
	return Found ? Found->Get() : nullptr;
}

void ABlackoutBossCharacter::OnDamageReceived(const FOnAttributeChangeData& Data)
{
	EvaluatePhaseTransition();
}

void ABlackoutBossCharacter::EvaluatePhaseTransition()
{
	if (!HasAuthority()) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;

	const float HP = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
	const float MaxHP = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());

	if (MaxHP <= 0.f) return;

	const float Percent = HP / MaxHP;

	ABlackoutBossAIController* AICon = Cast<ABlackoutBossAIController>(GetController());
	if (!AICon) return;

	if (Percent <= 0.5f)
	{
		AICon->RequestPhaseChange(EBossPhase::Phase2);
	}
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
