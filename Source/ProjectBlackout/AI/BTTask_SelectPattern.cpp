#include "AI/BTTask_SelectPattern.h"
#include "AI/BOPhasePatternData.h"
#include "AI/BTNodeHelper.h"
#include "AI/BOAICalcHelper.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogSelectPattern, Log, All);

UBTTask_SelectPattern::UBTTask_SelectPattern()
{
	NodeName = "Select Pattern";
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_SelectPattern::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!PatternData)
	{
		UE_LOG(LogSelectPattern, Error, TEXT("PatternData가 설정되지 않았습니다."));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);
	if (!BB) return EBTNodeResult::Failed;

	const float Distance = ResolveDistance(OwnerComp);

	TArray<const FBOPatternEntry*> Candidates = FilterCandidates(Distance);
	if (Candidates.IsEmpty())
	{
		UE_LOG(LogSelectPattern, Verbose, TEXT("유효한 패턴 없음 (거리: %.0fcm)"), Distance);
		return EBTNodeResult::Failed;
	}

	const FGameplayTag Selected = PickWeighted(Candidates);
	if (!Selected.IsValid()) return EBTNodeResult::Failed;

	BB->SetValueAsName(SelectedTagKey.SelectedKeyName, Selected.GetTagName());

	for (const FBOPatternEntry* Entry : Candidates)
	{
		if (Entry->AbilityTag != Selected) continue;

		StartCooldown(Selected, Entry->Cooldown);

		// 접근 거리 + 패턴 최대 거리 BB에 기록
		if (ApproachDistanceKey.SelectedKeyName != NAME_None)
		{
			const float ApproachDist = CalcApproachDistance(Distance, Entry->MinDistance, Entry->MaxDistance);
			BB->SetValueAsFloat(ApproachDistanceKey.SelectedKeyName, ApproachDist);
		}

		if (MaxDistanceKey.SelectedKeyName != NAME_None)
		{
			BB->SetValueAsFloat(MaxDistanceKey.SelectedKeyName, Entry->MaxDistance);
		}

		break;
	}

	UE_LOG(LogSelectPattern, Verbose, TEXT("패턴 선택: %s (거리: %.0fcm)"), *Selected.ToString(), Distance);
	return EBTNodeResult::Succeeded;
}

// ─────────────────────────────────────────────────────────────────────────────

TArray<const FBOPatternEntry*> UBTTask_SelectPattern::FilterCandidates(float Distance) const
{
	TArray<const FBOPatternEntry*> Result;
	for (const FBOPatternEntry& Entry : PatternData->Patterns)
	{
		// MinDistance 미만만 하드 컷오프.
		// MaxDistance 초과는 선택 가능 — 접근 후 공격하면 되므로.
		if (Distance < Entry.MinDistance) continue;
		if (CooldownHandles.Contains(Entry.AbilityTag)) continue;
		Result.Add(&Entry);
	}
	return Result;
}

float UBTTask_SelectPattern::CalcApproachDistance(
	float CurrentDistance, float MinDist, float MaxDist) const
{
	if (CurrentDistance > MaxDist)
	{
		return MaxDist;
	}
	return FMath::FRandRange(MinDist, CurrentDistance);
}

FGameplayTag UBTTask_SelectPattern::PickWeighted(const TArray<const FBOPatternEntry*>& Candidates) const
{
	float TotalWeight = 0.f;
	for (const FBOPatternEntry* Entry : Candidates)
	{
		TotalWeight += Entry->Weight;
	}

	if (TotalWeight <= 0.f) return FGameplayTag::EmptyTag;

	float Roll = FMath::FRandRange(0.f, TotalWeight);
	float Accumulated = 0.f;

	for (const FBOPatternEntry* Entry : Candidates)
	{
		Accumulated += Entry->Weight;
		if (Roll < Accumulated)
		{
			return Entry->AbilityTag;
		}
	}

	return Candidates.Last()->AbilityTag;
}

void UBTTask_SelectPattern::StartCooldown(const FGameplayTag& Tag, float Duration)
{
	if (Duration <= 0.f) return;

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateWeakLambda(this, [this, Tag]()
		{
			CooldownHandles.Remove(Tag);
		}),
		Duration,
		false
	);

	CooldownHandles.Add(Tag, Handle);
}

float UBTTask_SelectPattern::ResolveDistance(UBehaviorTreeComponent& OwnerComp) const
{
	UBlackboardComponent* BB = UBTNodeHelper::GetBlackboard(OwnerComp);

	if (BB && DistanceKey.SelectedKeyName != NAME_None)
	{
		const float BBDist = BB->GetValueAsFloat(DistanceKey.SelectedKeyName);
		if (BBDist > 0.f) return BBDist;
	}

	APawn* Pawn   = UBTNodeHelper::GetAIPawn(OwnerComp);
	AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName)) : nullptr;

	if (Pawn && Target)
	{
		return UBOAICalcHelper::GetDistance2D(Pawn, Target);
	}

	UE_LOG(LogSelectPattern, Warning, TEXT("거리 계산 실패: Pawn 또는 Target이 유효하지 않습니다."));
	return 0.f;
}

FString UBTTask_SelectPattern::GetStaticDescription() const
{
	return FString::Printf(TEXT("Select Pattern  |  Data: %s"),
		PatternData ? *PatternData->GetName() : TEXT("None"));
}
