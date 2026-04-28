#include "BlackoutEnemyCharacter.h"

#include "AudioMixerBlueprintLibrary.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Attributes/BlackoutBaseAttributeSet.h"
#include "Data/BOMinionData.h"
#include "Components/CapsuleComponent.h"
#include "BlackoutLog.h"

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

		// 기본 AttributeSet 추가 및 MinionData 수치 주입
		if (BaseAttributeSet && MinionData)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
				MinionData->MaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetHealthAttribute(),
				MinionData->MaxHealth);
		}

		// 데이터 에셋에 추가된 GA 일괄 부여 (서버 권한 가드는 GiveDefaultAbilities 내부 처리)
		if (MinionData && MinionData->GrantedAbilities.Num() > 0)
		{
			AbilitySystemComponent->GiveDefaultAbilities(
				MinionData->GrantedAbilities);
		}
	}
}

void ABlackoutEnemyCharacter::OnSpawnFromPool_Implementation()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// HP / GA 둘다 재초기화
	if (AbilitySystemComponent && MinionData)
	{
		if (BaseAttributeSet)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
				MinionData->MaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetHealthAttribute(),
				MinionData->MaxHealth);
		}
		if (MinionData->GrantedAbilities.Num() > 0)
		{
			AbilitySystemComponent->GiveDefaultAbilities(
				MinionData->GrantedAbilities);
		}
	}


	BO_LOG_POOL(Verbose, "SpawnFromPool: %s", *GetName());
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

	BO_LOG_POOL(Verbose, "ReturnToPool: %s", *GetName());
}
