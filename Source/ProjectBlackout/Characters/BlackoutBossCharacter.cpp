#include "BlackoutBossCharacter.h"
#include "AIController.h"
#include "BlackoutBaseAttributeSet.h"
#include "BlackoutBossAIController.h"
#include "BrainComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/PlayerState.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "UI/BlackoutHUD.h"
#include "UI/BlackoutEnemyHUDWidgetController.h"

ABlackoutBossCharacter::ABlackoutBossCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
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

void ABlackoutBossCharacter::SetData()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	for (const auto& [Tag, Data] : BossPatternData)
	{
		if (!Data || !Data->GrantedAbility) continue;
		ASC->GiveAbility(FGameplayAbilitySpec(Data->GrantedAbility, 1));
	}
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (BaseAttributeSet && BossData)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
				BossData->MaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetHealthAttribute(),
				BossData->MaxHealth);
		}
	}
}

void ABlackoutBossCharacter::OnDamageReceived(const FOnAttributeChangeData& Data)
{
	const float DamageDealt = Data.OldValue - Data.NewValue;
	if (DamageDealt <= 0.f || !Data.GEModData) return;
	
	ABlackoutBossAIController* AIC = Cast<ABlackoutBossAIController>(GetController());
	if (!AIC) return;
	
	AActor* SourceActor = Data.GEModData->EffectSpec.GetContext().GetInstigator();
	if (APawn* InstigatorPawn = ResolveInstigatorPawn(SourceActor))
	{
		if (InstigatorPawn != this)
		{
			AIC->RecordDamage(InstigatorPawn, DamageDealt);
		}
	}
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const float MaxHP = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
		if (MaxHP > 0.f)
		{
			const float CurrentHP = Data.NewValue;
			const float HealthRatio = CurrentHP / MaxHP;
	
			const EBOBossPhase TargetPhase = DetermineTargetPhase(HealthRatio);
			AIC->RequestPhaseChange(TargetPhase);
		}
	}
}

FText ABlackoutBossCharacter::GetBossDisplayName() const
{
	if (BossData->IsValid())
	{
		return BossData->Name;
	}
	return FText::FromString(TEXT("Ravager"));
}


void ABlackoutBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	SetData();

	// for (const auto& [_, Data] : BossAbilityData)
	// {
	// 	if (!Data || !Data->GrantedAbility) continue;
	//
	// 	ASC->GiveAbility(FGameplayAbilitySpec(Data->GrantedAbility, 1));
	// }

	if (HasAuthority())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			   UBlackoutBaseAttributeSet::GetHealthAttribute())
		   .AddUObject(this, &ABlackoutBossCharacter::OnDamageReceived);
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		this, &ABlackoutBossCharacter::TryBindToHUD);
}


UBORavagerPatternData* ABlackoutBossCharacter::GetPatternData(FGameplayTag AbilityTag) const
{
	const TObjectPtr<UBORavagerPatternData>* Found = BossPatternData.Find(AbilityTag);
	return Found ? Found->Get() : nullptr;
}


EBOBossPhase ABlackoutBossCharacter::DetermineTargetPhase(float HealthRatio)
{
	return HealthRatio <= 0.5f ? EBOBossPhase::Phase2 : EBOBossPhase::Phase1;
}

APawn* ABlackoutBossCharacter::ResolveInstigatorPawn(AActor* SourceActor) const
{
	if (!SourceActor) return nullptr;

	if (APawn* Pawn = Cast<APawn>(SourceActor)) return Pawn;
	if (AController* C = Cast<AController>(SourceActor)) return C->GetPawn();
	if (APlayerState* PS = Cast<APlayerState>(SourceActor)) return PS->GetPawn();

	return nullptr;
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
				EnemyHUDController->BindToEnemy(GetAbilitySystemComponent(), GetBossDisplayName());
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
