#pragma once

#include "CoreMinimal.h"
#include "AI/BlackoutAIController.h"
#include "BlackoutBossAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;

/**
 * 보스 전용 AI 컨트롤러.
 * StateTree로 페이즈를 관리하고, 페이즈별 하위 BehaviorTree로 전투 패턴을 수행.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutBossAIController : public ABlackoutAIController
{
	GENERATED_BODY()

public:
	ABlackoutBossAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnPossess(APawn* InPawn) override;

	/** 현재 페이즈에 맞는 하위 BehaviorTree를 실행 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void RunSubBehaviorTree(UBehaviorTree* SubTree);

	/** 현재 실행 중인 하위 BehaviorTree를 정지 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void StopSubBehaviorTree();

	/** 어그로 Evaluator가 선정한 타겟을 Blackboard에 기록 */
	UFUNCTION(BlueprintCallable, Category = "Blackout|AI")
	void WriteTargetToBlackboard(APawn* TargetPawn);

protected:
	virtual void InitStateTreeContext() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UBehaviorTreeComponent> SubBehaviorTreeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UBlackboardComponent> BlackboardComp;
};
