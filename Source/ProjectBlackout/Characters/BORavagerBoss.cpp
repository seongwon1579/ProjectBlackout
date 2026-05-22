#include "Characters/BORavagerBoss.h"

#include "AbilitySystemComponent.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutBaseAttributeSet.h"
#include "Abilities/GameplayAbility.h"


// ABORavagerBoss::ABORavagerBoss()
// {
// 	PrimaryActorTick.bCanEverTick = true;
// }
//
// UDataAsset* ABORavagerBoss::GetPatternData(FGameplayTag AbilityTag) const
// {
// 	const TObjectPtr<UBORavagerPatternData>* Found = BossPatternData.Find(AbilityTag);
// 	return Found ? Found->Get() : nullptr;
// }
//
// void ABORavagerBoss::SetData()
// {
// 	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
// 	if (!ASC) return;
// 	
// 	for (const auto& [Tag, Data] : BossPatternData)
// 	{
// 		if (!Data || !Data->GrantedAbility) continue;
// 		ASC->GiveAbility(FGameplayAbilitySpec(Data->GrantedAbility, 1));
// 	}
// 	
// 	if (AbilitySystemComponent)
// 	{
// 		AbilitySystemComponent->InitAbilityActorInfo(this, this);
//
// 		if (BaseAttributeSet && BossData)
// 		{
// 			AbilitySystemComponent->SetNumericAttributeBase(
// 				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
// 				BossData->MaxHealth);
// 			AbilitySystemComponent->SetNumericAttributeBase(
// 				UBlackoutBaseAttributeSet::GetHealthAttribute(),
// 				BossData->MaxHealth);
// 		}
// 	}
// }
//
// EBOBossPhase ABORavagerBoss::DetermineTargetPhase(float HealthRatio) const
// {
// 	return HealthRatio <= 0.5f ? EBOBossPhase::Phase2 : EBOBossPhase::Phase1;
// }
//
// FText ABORavagerBoss::GetBossDisplayName() const
// {
// 	if (BossData->IsValid())
// 	{
// 		return BossData->Name;
// 	}
// 	return FText::FromString(TEXT("Ravager"));
// }
