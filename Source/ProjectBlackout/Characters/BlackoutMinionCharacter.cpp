#include "BlackoutMinionCharacter.h"

#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutLog.h"
#include "BODissolveComponent.h"
#include "BOMinionHealthBarComponent.h"
#include "Attributes/BlackoutBaseAttributeSet.h"
#include "Data/BOMinionData.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

ABlackoutMinionCharacter::ABlackoutMinionCharacter()
{
	DissolveComponent = CreateDefaultSubobject<UBODissolveComponent>(TEXT("DissolveComponent"));
	HealthBarComponent = CreateDefaultSubobject<UBOMinionHealthBarComponent>(TEXT("HealthBarComponent"));
}

void ABlackoutMinionCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyMinionDataToASC();

	if (HealthBarComponent)
	{
		HealthBarComponent->InitializeFromASC(AbilitySystemComponent);
	}

	// 풀 액터 1회 바인딩: 서버 dissolve 완료 → 풀 반환.
	if (DissolveComponent)
	{
		DissolveComponent->OnServerDissolveFinished.AddUObject(
			this, &ABlackoutMinionCharacter::HandleDissolveFinished);
	}
}

void ABlackoutMinionCharacter::OnSpawnFromPool_Implementation()
{
	// 사망/다운 플래그 + 액터 가시성 / 충돌 / 틱 복구
	SetDeadStateActive(false);
	SetDownedStateActive(false);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// HP / GA 재초기화 + 체력바 재바인딩
	ApplyMinionDataToASC();
	if (HealthBarComponent)
	{
		HealthBarComponent->InitializeFromASC(AbilitySystemComponent);
	}

	// 사망 시 정지했던 movement / AI 복구
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetDefaultMovementMode();
	}
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->RestartLogic();
		}
	}

	if (DissolveComponent)
	{
		DissolveComponent->ResetDissolve();
	}

	BO_LOG_POOL(Verbose, "SpawnFromPool: %s", *GetName());
}

void ABlackoutMinionCharacter::OnReturnToPool_Implementation()
{
	if (HealthBarComponent)
	{
		HealthBarComponent->Hide();
	}

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

void ABlackoutMinionCharacter::OnDeath()
{
	const bool bWasAlreadyDead = IsDead();

	if (HealthBarComponent)
	{
		HealthBarComponent->Hide();
	}

	Super::OnDeath();

	if (!HasAuthority() || bWasAlreadyDead)
	{
		return;
	}

	SetActorEnableCollision(false);

	// 사망 후 행동 정지: AI(StateTree) + 이동 + 시전 중 어빌리티.
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Death"));
		}
	}
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DisableMovement();
	}
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAllAbilities();
	}

	if (DissolveComponent)
	{
		DissolveComponent->PlayDissolve();
	}
}

void ABlackoutMinionCharacter::ApplyMinionDataToASC()
{
	if (!AbilitySystemComponent || !BaseAttributeSet || !MinionData)
	{
		return;
	}

	AbilitySystemComponent->SetNumericAttributeBase(
		UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
		MinionData->MaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(
		UBlackoutBaseAttributeSet::GetHealthAttribute(),
		MinionData->MaxHealth);

	if (MinionData->GrantedAbilities.Num() > 0)
	{
		AbilitySystemComponent->GiveDefaultAbilities(MinionData->GrantedAbilities);
	}
}

void ABlackoutMinionCharacter::HandleDissolveFinished()
{
	if (UWorld* World = GetWorld())
	{
		if (UBlackoutPoolSubsystem* Pool = World->GetSubsystem<UBlackoutPoolSubsystem>())
		{
			Pool->ReturnToPool(this);
		}
	}
}
