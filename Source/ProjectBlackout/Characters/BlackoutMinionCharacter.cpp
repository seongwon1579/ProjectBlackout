#include "BlackoutMinionCharacter.h"

#include "BODissolveComponent.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

ABlackoutMinionCharacter::ABlackoutMinionCharacter()
{
	DissolveComponent = CreateDefaultSubobject<UBODissolveComponent>(TEXT("DissolveComponent"));
}

void ABlackoutMinionCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 풀 액터 1회 바인딩: 서버 dissolve 완료 → 풀 반환.
	if (DissolveComponent)
	{
		DissolveComponent->OnServerDissolveFinished.AddUObject(
			this, &ABlackoutMinionCharacter::HandleDissolveFinished);
	}
}

void ABlackoutMinionCharacter::OnDeath()
{
	const bool bWasAlreadyDead = IsDead();
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

void ABlackoutMinionCharacter::OnSpawnFromPool_Implementation()
{
	Super::OnSpawnFromPool_Implementation();

	// 사망 시 정지했던 상태 복구.
	ResetVitalState();
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->SetMovementMode(MOVE_Walking);
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
