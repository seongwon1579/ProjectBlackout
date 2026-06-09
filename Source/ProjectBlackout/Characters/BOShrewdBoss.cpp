#include "Characters/BOShrewdBoss.h"

#include "AIController.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutBaseAttributeSet.h"
#include "BrainComponent.h"
#include "GameplayEffectExtension.h"
#include "UBOShrewdData.h"
#include "GameFramework/PlayerState.h"
#include "AI/BlackoutAggroComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Framework/BlackoutMatchFlowSubsystem.h"

ABOShrewdBoss::ABOShrewdBoss()
{
	AggroComponent = CreateDefaultSubobject<UBlackoutAggroComponent>(
		TEXT("AggroComponent"));

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->DefaultLandMovementMode = MOVE_Flying;
		Move->GravityScale = 0.0f;
		Move->MaxFlySpeed = 300.0f;
		Move->MaxAcceleration = 2048.0f;
		Move->BrakingDecelerationFlying = 2048.0f;
	}
}

void ABOShrewdBoss::Multicast_DebugAggroTarget_Implementation(
	const FString& TargetName)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 3.0f, FColor::Yellow,
			FString::Printf(TEXT("Aggro Target: %s"), *TargetName));
	}
}

bool ABOShrewdBoss::GetRandomTeleportTransform(FTransform& OutTransform)
{
	TArray<AActor*> ValidPoints;
	for (const TObjectPtr<AActor>& Point : TeleportPoints)
	{
		if (IsValid(Point) && Point != LastTeleportPoint)
		{
			ValidPoints.Add(Point);
		}
	}

	if (ValidPoints.Num() == 0)
	{
		if (!IsValid(LastTeleportPoint)) return false;
		OutTransform = LastTeleportPoint->GetActorTransform();
		return true;
	}

	const int32 Index = FMath::RandRange(0, ValidPoints.Num() - 1);
	AActor* Selected = ValidPoints[Index];

	LastTeleportPoint = Selected;
	OutTransform = Selected->GetActorTransform();
	return true;
}

void ABOShrewdBoss::SetData()
{
	if (!AbilitySystemComponent || !BaseAttributeSet || !ShrewdData)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}


	float HealthMultiplier = 1.0f;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UBlackoutMatchFlowSubsystem* FlowSubsystem = GameInstance->
			GetSubsystem<UBlackoutMatchFlowSubsystem>())
		{
			HealthMultiplier = FlowSubsystem->GetBossHealthMultiplier();
		}
	}
	const float ScaledMaxHealth = ShrewdData->MaxHealth * HealthMultiplier;
	AbilitySystemComponent->SetNumericAttributeBase(
		UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
		ScaledMaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(
		UBlackoutBaseAttributeSet::GetHealthAttribute(),
		ScaledMaxHealth);

	if (ShrewdData->GrantedAbilities.Num() > 0)
	{
		AbilitySystemComponent->GiveDefaultAbilities(
			ShrewdData->GrantedAbilities);
	}
}

void ABOShrewdBoss::OnDamageReceived(const FOnAttributeChangeData& Data)
{
	const float DamageDealt = Data.OldValue - Data.NewValue;
	if (DamageDealt <= 0.f || !Data.GEModData) return;

	if (!AggroComponent)
	{
		return;
	}

	AActor* SourceActor = Data.GEModData->EffectSpec.GetContext().
	                           GetInstigator();
	if (APawn* InstigatorPawn = ResolveInstigatorPawn(SourceActor))
	{
		if (InstigatorPawn != this)
		{
			AggroComponent->RecordDamage(InstigatorPawn, DamageDealt);
		}
	}
}

APawn* ABOShrewdBoss::ResolveInstigatorPawn(AActor* SourceActor) const
{
	if (!SourceActor) return nullptr;

	if (APawn* Pawn = Cast<APawn>(SourceActor)) return Pawn;
	if (AController* C = Cast<AController>(SourceActor)) return C->GetPawn();
	if (APlayerState* PS = Cast<APlayerState>(SourceActor)) return PS->
		GetPawn();

	return nullptr;
}

void ABOShrewdBoss::OnDeath()
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

FText ABOShrewdBoss::GetBossDisplayName() const
{
	return FText::FromString(TEXT("Shrewd"));
}
