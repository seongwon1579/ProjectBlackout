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
	/** 패턴 목록 데이터 에셋. 에디터에서 페이즈별로 교체한다. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Pattern")
	TObjectPtr<UBOPhasePatternData> PatternData;

	/**
	 * BTService_UpdateTargetData가 기록하는 AI~타겟 2D 거리 키 (Float).
	 * 서비스가 없으면 직접 계산한다.
	 */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector DistanceKey;

	/** 선택된 어빌리티 태그 이름을 저장할 블랙보드 키 (Name). */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector SelectedTagKey;

	/** 타겟 액터를 읽어올 블랙보드 키 (Object). 거리 직접 계산 폴백에 사용. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * 접근 목표 거리를 저장할 블랙보드 키 (Float).
	 * BTTask_MoveToAttackRange에서 읽는다.
	 *
	 * CurrentDistance > MaxDist  → MaxDist 저장
	 * MinDist ≤ CurrentDistance  → Random(MinDist, CurrentDistance) 저장
	 */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector ApproachDistanceKey;

	/** 선택된 패턴의 MaxDistance를 저장할 블랙보드 키 (Float).
	 * BTTask_MoveToAttackRange에서 이동 중 이탈 감지에 사용한다. */
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard")
	FBlackboardKeySelector MaxDistanceKey;

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
