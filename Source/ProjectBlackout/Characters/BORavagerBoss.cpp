#include "Characters/BORavagerBoss.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutBaseAttributeSet.h"
#include "BlackoutBossAIController.h"
#include "BlackoutRavagerAIController.h"
#include "BrainComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/PlayerState.h"
#include "Framework/BlackoutMatchFlowSubsystem.h"


UBORavagerPatternData* ABORavagerBoss::GetPatternData(FGameplayTag AbilityTag) const
{
	const TObjectPtr<UBORavagerPatternData>* Found = BossPatternData.Find(AbilityTag);
	return Found ? Found->Get() : nullptr;
}

void ABORavagerBoss::SetCollisionState(bool bIgnore)
{
	if (HasAuthority())
	{
		Multicast_SetCollisionState(bIgnore);
	}
}

void ABORavagerBoss::Multicast_SetCollisionState_Implementation(bool bIgnore)
{
	BP_OnCollisionStateChanged(bIgnore);
}

void ABORavagerBoss::OnDeath()
{
	Super::OnDeath();
	
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->CancelAllAbilities();
	}
	
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AIC->GetBrainComponent())
		{
			Brain->StopLogic("Dead");
		}
	}
}

void ABORavagerBoss::SetData()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	if (HasAuthority())
	{
		for (const auto& [Tag, Data] : BossPatternData)
		{
			if (!Data || !Data->GrantedAbility) continue;
			ASC->GiveAbility(FGameplayAbilitySpec(Data->GrantedAbility, 1));
		}
	}
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (BaseAttributeSet && BossStatData && HasAuthority())
		{
			float HealthMultiplier =1.0f;
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (UBlackoutMatchFlowSubsystem* FlowSubsystem = GameInstance->GetSubsystem<UBlackoutMatchFlowSubsystem>())
				{
					HealthMultiplier = FlowSubsystem->GetBossHealthMultiplier();
				}
			}
			
			const float ScaledMaxHealth = BossStatData->MaxHealth * HealthMultiplier;
			
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
				ScaledMaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetHealthAttribute(),
				ScaledMaxHealth);
		}
	}
}

void ABORavagerBoss::OnDamageReceived(const FOnAttributeChangeData& Data)
{
	const float DamageDealt = Data.OldValue - Data.NewValue;
	if (DamageDealt <= 0.f || !Data.GEModData) return;
	
	ABlackoutRavagerAIController* AIC = Cast<ABlackoutRavagerAIController>(GetController());
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

FText ABORavagerBoss::GetBossDisplayName() const
{
	if (BossStatData->IsValid())
	{
		return BossStatData->Name;
	}
	return FText::FromString(TEXT("Ravager"));
}

EBOBossPhase ABORavagerBoss::DetermineTargetPhase(float HealthRatio)
{
	if (HealthRatio <= 0.3f)
	{
		return EBOBossPhase::Phase3;
	}
	
	if (HealthRatio <= 0.6f)
	{
		return EBOBossPhase::Phase2;
	}
	
	return EBOBossPhase::Phase1;
}

APawn* ABORavagerBoss::ResolveInstigatorPawn(AActor* SourceActor) const
{
	if (!SourceActor) return nullptr;

	if (APawn* Pawn = Cast<APawn>(SourceActor)) return Pawn;
	if (AController* C = Cast<AController>(SourceActor)) return C->GetPawn();
	if (APlayerState* PS = Cast<APlayerState>(SourceActor)) return PS->GetPawn();

	return nullptr;
}
