#include "BlackoutEnemyCharacter.h"

#include "BlackoutAbilitySystemComponent.h"
#include "Attributes/BlackoutBaseAttributeSet.h"

ABlackoutEnemyCharacter::ABlackoutEnemyCharacter()
{
	// 적은 자기 자신이 ASC를 소유
	AbilitySystemComponent = CreateDefaultSubobject<
		UBlackoutAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(
		EGameplayEffectReplicationMode::Minimal);

	BaseAttributeSet = CreateDefaultSubobject<UBlackoutBaseAttributeSet>(
		TEXT("BaseAttributeSet"));
	AbilitySystemComponent->AddAttributeSetSubobject(BaseAttributeSet.Get());
}

void ABlackoutEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
