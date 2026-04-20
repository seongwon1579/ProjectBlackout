#include "BlackoutEnemyCharacter.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Attributes/BlackoutBaseAttributeSet.h"
#include "Data/BOMinionData.h"
#include "Components/CapsuleComponent.h"
#include "BlackoutLogCategories.h"

ABlackoutEnemyCharacter::ABlackoutEnemyCharacter()
{
	// 적은 자기 자신이 ASC를 소유
	AbilitySystemComponent = CreateDefaultSubobject<UBlackoutAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

void ABlackoutEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 기본 AttributeSet 추가 및 MinionData 수치 주입
		UBlackoutBaseAttributeSet* AttrSet = AbilitySystemComponent->GetSet<UBlackoutBaseAttributeSet>();
		if (AttrSet && MinionData)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(), MinionData->MaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetHealthAttribute(), MinionData->MaxHealth);
		}
	}
}

void ABlackoutEnemyCharacter::OnSpawnFromPool_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// HP를 최대치로 복구
	if (AbilitySystemComponent && MinionData)
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			UBlackoutBaseAttributeSet::GetHealthAttribute(), MinionData->MaxHealth);
	}

	UE_LOG(LogBlackoutPool, Verbose, TEXT("SpawnFromPool: %s"), *GetName());
}

void ABlackoutEnemyCharacter::OnReturnToPool_Implementation()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// GE·GA 전체 제거 (다음 스폰 시 깨끗한 상태 보장)
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ClearAllAbilitiesAndEffects();
	}

	UE_LOG(LogBlackoutPool, Verbose, TEXT("ReturnToPool: %s"), *GetName());
}
