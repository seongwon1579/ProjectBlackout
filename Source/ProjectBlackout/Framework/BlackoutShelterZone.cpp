#include "BlackoutShelterZone.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BlackoutBattleGameMode.h"
#include "BlackoutGameState.h"
#include "BlackoutLog.h"
#include "BlackoutPlayerState.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameplayTags/BlackoutGameplayTags.h"

ABlackoutShelterZone::ABlackoutShelterZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn,ECR_Overlap);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	Trigger->SetGenerateOverlapEvents(true);
}

void ABlackoutShelterZone::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &ABlackoutShelterZone::OnTriggerBegin);
	OnActorEndOverlap.AddDynamic(this, &ABlackoutShelterZone::OnTriggerEnd);

	SnapshotCurrentOverlaps();
	if (IsTargetPhaseActive())
	{
		for (auto It = OverlappingPawns.CreateIterator(); It; ++It)
		{
			if (APawn* Pawn = It->Get())
			{
				ApplyShelterEffects(Pawn);
			}
		}
	}
	if (ABlackoutGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr)
	{
		GameState->OnMatchStateChanged.AddDynamic(this, &ABlackoutShelterZone::HandleMatchStateChanged);
	}
}

bool ABlackoutShelterZone::IsTargetPhaseActive() const
{
	const ABlackoutGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	return GS && GS->CurrentMatchState == TargetShelterPhase;
}

void ABlackoutShelterZone::SnapshotCurrentOverlaps()
{
	if (!HasAuthority())
	{
		return;
	}
	
	TArray<AActor*> CurrentlyOverlapping;
	GetOverlappingActors(CurrentlyOverlapping,APawn::StaticClass());
	for (AActor* Actor : CurrentlyOverlapping)
	{
		if (APawn* Pawn = Cast<APawn>(Actor))
		{
			OverlappingPawns.Add(Pawn);
		}
	}
}

void ABlackoutShelterZone::OnTriggerBegin(AActor* /*OverlappedActor*/, AActor* OtherActor)
{
	if (!HasAuthority())
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	OverlappingPawns.Add(Pawn);
	if (IsTargetPhaseActive())
	{
		ApplyShelterEffects(Pawn);
	}
}

void ABlackoutShelterZone::OnTriggerEnd(AActor* /*OverlappedActor*/, AActor* OtherActor)
{
	if (!HasAuthority())
	{
		return;
	}

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn)
	{
		return;
	}

	OverlappingPawns.Remove(Pawn);
	RemoveShelterEffects(Pawn);
}

void ABlackoutShelterZone::HandleMatchStateChanged(EBlackoutMatchState NewState)
{
	if (!HasAuthority())
	{
		return;
	}

	const bool bEnteringTarget = (NewState == TargetShelterPhase);

	if (bEnteringTarget)
	{
		SnapshotCurrentOverlaps();
	}
	
	for (auto It = OverlappingPawns.CreateIterator(); It; ++It)
	{
		APawn* Pawn = It->Get();
		if (!Pawn)
		{
			It.RemoveCurrent();
			continue;
		}
		
		if (bEnteringTarget)
		{
			ApplyShelterEffects(Pawn);
		}
		else
		{
			RemoveShelterEffects(Pawn);
		}
	}
}

void ABlackoutShelterZone::ApplyShelterEffects(APawn* Pawn)
{
	if (!Pawn)
	{
		return;
	}

	ABlackoutPlayerState* PS = Pawn->GetPlayerState<ABlackoutPlayerState>();
	if (!PS)
	{
		return;
	}
	
	// PS ASC 사용 — PossessedBy 의존 제거, GA ActorInfo와 동일 ASC.
	if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
	{
		if (ShelterStateEffectClass)
		{
			FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ShelterStateEffectClass , 1.0f, Ctx);
			if (SpecHandle.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}

	PS->ApplyBattleTransitionPolicy(EBattleTransitionType::CheckpointRest);

	if (ABlackoutBattleGameMode* GM = GetWorld()->GetAuthGameMode<ABlackoutBattleGameMode>())
	{
		GM->HandleCheckpoint(this);
	}

	BO_LOG_NET(Log, "ShelterZone Enter: %s @ %s", *GetNameSafe(PS), *GetName());
}

void ABlackoutShelterZone::RemoveShelterEffects(APawn* Pawn)
{
	if (!Pawn)
	{
		return;
	}
	
	ABlackoutPlayerState* PS = Pawn->GetPlayerState<ABlackoutPlayerState>();
	if (!PS)
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
	if (!ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_InShelter))
	{
		return;
	}
	
	FGameplayTagContainer InShelterTags;
	InShelterTags.AddTag(BlackoutGameplayTags::State_InShelter);
	ASC->RemoveActiveEffectsWithGrantedTags(InShelterTags);
	
	FGameplayTagContainer ScopedTags;
	ScopedTags.AddTag(BlackoutGameplayTags::Effect_ShelterScoped);
	ASC->RemoveActiveEffectsWithGrantedTags(ScopedTags);
	
	BO_LOG_NET(Log, "ShelterZone Exit: %s @ %s", *GetNameSafe(Pawn), *GetName());
}
