#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTTask_SelectPattern.generated.h"

struct FBOPatternEntry;
class UBOPhasePatternData;

/**
 * 현재 거리와 쿨다운을 기준으로 실행 가능한 패턴을 필터링한 뒤
 * 가중치 랜덤으로 하나를 선택해 블랙보드에 기록하는 BT Task.
 *
 * - bCreateNodeInstance = true: 쿨다운 상태(CooldownHandles)를 인스턴스 멤버로 유지.
 * - Tick 없음: 쿨다운 해제는 엔진 타이머로 처리.
 * - 유효한 후보가 없으면 Failed → BT가 다른 노드로 폴백한다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBTTask_SelectPattern : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_SelectPattern();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString             GetStaticDescription() const override;

protected:
	
	// Data
	UPROPERTY(EditAnywhere, Category = "Blackout|Data")
	TObjectPtr<UBOPhasePatternData> PatternData;
	
	// Input Key
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector DistanceBTWActorsKey;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector SelectedGamePlayTagKey;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector CurrentTargetKey;
	
	// Output Key
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Output")
	FBlackboardKeySelector ApproachDistanceKey;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Output")
	FBlackboardKeySelector MaxDistanceKey;

	/** 현재 거리가 패턴 MaxDistance를 초과할 때 true — 쫓아가는 패턴임을 표시 */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Output")
	FBlackboardKeySelector bIsChasingKey;

private:
	/** 현재 쿨다운 중인 패턴. 타이머 만료 시 자동 제거. */
	TMap<FGameplayTag, FTimerHandle> CooldownHandles;

	/**
	 * MinDist 미만만 제외. MaxDist 초과는 선택 가능 (접근 후 공격).
	 * 쿨다운 중인 패턴도 제외한다.
	 */
	TArray<const FBOPatternEntry*> FilterCandidates(float Distance) const;

	/** 가중치 비례 랜덤으로 태그를 선택한다. */
	FGameplayTag PickWeighted(const TArray<const FBOPatternEntry*>& Candidates) const;

	/** Duration 초 동안 해당 태그를 쿨다운 상태로 등록한다. */
	void StartCooldown(const FGameplayTag& Tag, float Duration);

	/** BB 또는 액터 직접 계산으로 현재 거리를 구한다. */
	float ResolveDistance(UBehaviorTreeComponent& OwnerComp) const;

	/**
	 * 선택된 패턴의 거리 범위와 현재 거리를 기준으로 접근 목표 거리를 계산한다.
	 *
	 * CurrentDistance > MaxDist  → MaxDist
	 * MinDist ≤ CurrentDistance  → Random(MinDist, CurrentDistance)
	 */
	float CalcApproachDistance(float CurrentDistance, float MinDist, float MaxDist) const;
};
