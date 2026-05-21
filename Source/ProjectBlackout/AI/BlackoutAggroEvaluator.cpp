// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutAggroEvaluator.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "BlackoutBaseAttributeSet.h"
#include "BlackoutGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"

UBlackoutAggroEvaluator::UBlackoutAggroEvaluator()
{
}

UWorld* UBlackoutAggroEvaluator::GetWorld() const
{
	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

void UBlackoutAggroEvaluator::Initialize(AAIController* InAIController, UAbilitySystemComponent* InASC)
{
	CachedOwnerAIController = InAIController;
	CachedASC = InASC;

	if (CachedASC)
	{
		TargetChangeTag = BlackoutGameplayTags::Ability_TargetChange;

		TargetChangeTagChangedHandle = CachedASC->RegisterGameplayTagEvent(
			TargetChangeTag,
			EGameplayTagEventType::NewOrRemoved
		).AddUObject(this, &UBlackoutAggroEvaluator::OnAggroTargetChangeTagChanged);
	}

	UpdateTarget();
}

void UBlackoutAggroEvaluator::Deinitialize()
{
	if (CachedASC)
	{
		CachedASC->RegisterGameplayTagEvent(
			TargetChangeTag,
			EGameplayTagEventType::NewOrRemoved
		).Remove(TargetChangeTagChangedHandle);
	}

	CachedASC = nullptr;
	CachedOwnerAIController = nullptr;
	CombatDataMap.Empty();
}

void UBlackoutAggroEvaluator::UpdateTarget()
{
	if (!CachedOwnerAIController) return;

	UBlackboardComponent* BB = CachedOwnerAIController->GetBlackboardComponent();
	APawn* BestTarget = CalculateBestTarget(nullptr);

	//TODO: 현재는 무한 반복으로 BestTarget이 세팅될 때까지 기다리지만 이 후에 보스방 입장 트리거로 Pawn에 입장을 확인하던지 perception으로 확인하던지 변경 필요
	if (!BB || !BestTarget)
	{
		CachedOwnerAIController->GetWorldTimerManager().SetTimerForNextTick(
			this, &UBlackoutAggroEvaluator::UpdateTarget);
		return;
	}

	BB->SetValueAsObject(TEXT("Target"), BestTarget);
}

void UBlackoutAggroEvaluator::OnAggroTargetChangeTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount == 0)
	{
		UpdateTarget();
	}
}

void UBlackoutAggroEvaluator::RecordDamage(APawn* Source, float Amount)
{
	if (!IsValid(Source) || Amount <= 0) return;

	FPlayerCombatData& Data = CombatDataMap.FindOrAdd(Source);
	FDamageRecord NewRecord;
	NewRecord.Timestamp = GetWorld()->GetTimeSeconds();
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
