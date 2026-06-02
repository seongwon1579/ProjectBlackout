// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutAggroEvaluator.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "BlackoutBaseAttributeSet.h"
#include "BlackoutGameplayTags.h"
#include "GameFramework/GameModeBase.h"

UWorld* UBlackoutAggroEvaluator::GetWorld() const
{
	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

void UBlackoutAggroEvaluator::Initialize(AAIController* InAIController, UAbilitySystemComponent* InASC)
{
	CachedOwnerAIController = InAIController;
	CachedASC = InASC;
	
	if (!InAIController || !InAIController->HasAuthority()) return;
	
	RegisterTagEvents();
	RegisterPlayerEvents();
	
	UpdateTarget();
}

void UBlackoutAggroEvaluator::Deinitialize()
{
	UnregisterTagEvents();
	UnregisterPlayerEvents();
	
	CachedASC = nullptr;
	CachedOwnerAIController = nullptr;
	CombatDataMap.Empty();
}

void UBlackoutAggroEvaluator::UpdateTarget()
{
	if (!CachedOwnerAIController) return;

	APawn* BestTarget = CalculateBestTarget(nullptr);
	
	if (BestTarget)
	{
		OnAggroTargetChanged.Broadcast(BestTarget);
	}
}

void UBlackoutAggroEvaluator::OnAggroTargetChangeTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount == 0)
	{
		UpdateTarget();
	}
}

void UBlackoutAggroEvaluator::OnPlayerPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (NewPawn)
	{
		UpdateTarget();
	}
}

void UBlackoutAggroEvaluator::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPC)
{
	if (!NewPC) return;

	NewPC->OnPossessedPawnChanged.AddDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);

	// 접속하자마자 Pawn을 들고 있는 경우도 있음
	if (NewPC->GetPawn())
	{
		UpdateTarget();
	}
}

void UBlackoutAggroEvaluator::RecordDamage(APawn* Source, float Amount)
{
	if (!IsValid(Source) || Amount <= 0) return;

	FPlayerCombatData& Data = CombatDataMap.FindOrAdd(Source);
	FDamageRecord NewRecord;
	
	UWorld* World = GetWorld();
	if (!World) return;  
	
	NewRecord.Timestamp = World->GetTimeSeconds();
	NewRecord.Amount = Amount;
	Data.DamageRecords.Add(NewRecord);
}

APawn* UBlackoutAggroEvaluator::CalculateBestTarget(APawn* ExcludeTarget) const
{
	if (!CachedOwnerAIController) return nullptr;

	APawn* Owner = CachedOwnerAIController->GetPawn();
	if (!Owner || !GetWorld()) return nullptr;

	APawn* BestTarget = nullptr;

	float BestScore = -1.f;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* Target = PC->GetPawn();
		if (!IsValid(Target)) continue;

		// 최대사거리를 벗어난 Target은 제외
		const float DistSQ = FVector::DistSquared(Target->GetActorLocation(), Owner->GetActorLocation());
		if (DistSQ > MaxAggroRange * MaxAggroRange) continue;

		const float Score = CalculateAggroScore(Target);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}
	return BestTarget;
}

void UBlackoutAggroEvaluator::RegisterTagEvents()
{
	if (!CachedASC) return;
	
	TargetChangeTag = BlackoutGameplayTags::Ability_TargetChange;
	TargetChangeTagChangedHandle = CachedASC->RegisterGameplayTagEvent(
		TargetChangeTag,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UBlackoutAggroEvaluator::OnAggroTargetChangeTagChanged);
}

void UBlackoutAggroEvaluator::UnregisterTagEvents()
{
	if (!CachedASC) return;
	
	CachedASC->RegisterGameplayTagEvent(
		TargetChangeTag,
		EGameplayTagEventType::NewOrRemoved
	).Remove(TargetChangeTagChangedHandle);
}

void UBlackoutAggroEvaluator::RegisterPlayerEvents()
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->OnPossessedPawnChanged.AddDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);
		}
	}
	
	PostLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(
		this, &UBlackoutAggroEvaluator::OnPostLogin);
}

void UBlackoutAggroEvaluator::UnregisterPlayerEvents()
{
	FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginHandle);
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->OnPossessedPawnChanged.RemoveDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);
		}
	}
}


float UBlackoutAggroEvaluator::CalculateAggroScore(APawn* Target) const
{
	if (!IsValid(Target) || !CachedOwnerAIController) return 0.f;

	APawn* Owner = CachedOwnerAIController->GetPawn();
	if (!Owner || !GetWorld()) return 0.f;

	float Score = 0.f;
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// 거리
	const float Dist = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
	const float DistScore = FMath::Clamp(1.f - (Dist / MaxAggroRange), 0.f, 1.f);
	Score += DistScore * DistanceWeight;

	// DPS
	if (FPlayerCombatData* Data = CombatDataMap.Find(Target))
	{
		const float DamageInWindow = Data->GetDamageInWindow(CurrentTime, DPSWindowDuration);
		const float DPS = DamageInWindow / DPSWindowDuration;
		const float DPSScore = FMath::Min(DPS / 100.f, 1.f);
		// TODO: 커브로 변경 필요
		// if (DPSAggroCurve)
		// {
		// 	DPSScore =
		// 		DPSAggroCurve->GetFloatValue(DPS);
		// }
		Score += DPSScore * DPSWeight;
	}

	// 체력
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const float HP = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
			const float MaxHP = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());

			if (MaxHP > 0.f)
			{
				Score += (1.f - HP / MaxHP) * LowHPWeight;
			}
		}
	}
	return Score;
}
