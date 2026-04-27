#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BossBTRunner.generated.h"

class ABlackoutBossAIController;
class UBehaviorTree;
class UBehaviorTreeComponent;
class UBlackboardComponent;
class APawn;

/**
 * 보스 AI의 BehaviorTree(전투 패턴) 생명주기를 담당한다.
 * - 페이즈별 SubBehaviorTree 실행 / 정지
 * - Blackboard 데이터 기록 (타겟, 페이즈 탈출 플래그)
 * - IsRunning: FBSTTask_RunSubBehaviorTree::Tick 이 BT 자체 종료를 감지하는 데 사용
 *
 * 서버 전용(Dedicated Server): Authority 검증 후 실행.
 * BehaviorTreeComponent / BlackboardComponent 는 컨트롤러(Actor)에 소속되며,
 * 이 객체는 그것에 대한 로직 위임자다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBossBTRunner : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(ABlackoutBossAIController* InOwner,
	                UBehaviorTreeComponent*    InBTComp,
	                UBlackboardComponent*      InBBComp);

	/** 페이즈에 맞는 SubBehaviorTree를 (재)시작한다. InitialTarget이 있으면 BB 초기화 후 즉시 주입한다. */
	void RunBehaviorTree(UBehaviorTree* SubTree, APawn* InitialTarget = nullptr);

	/** BT를 즉시 안전하게 정지한다. */
	void Stop();

	/** BT가 현재 실행 중인지 여부. */
	bool IsRunning() const;

	/** 어그로 시스템이 선정한 타겟을 Blackboard에 기록한다. */
	void WriteTargetToBlackboard(APawn* TargetPawn);

	/**
	 * 페이즈 전환을 요청한다.
	 * BT를 즉시 중단하지 않고 BB_RequestPhaseExit 를 설정해
	 * BTTask_CheckPhaseExit 가 현재 어빌리티 완료 후 안전하게 BT를 종료하도록 유도한다.
	 */
	void RequestPhaseExit();
	
	AActor* CheckingActor;

private:
	bool HasAuthority() const;

	TWeakObjectPtr<ABlackoutBossAIController> OwnerController;
	TWeakObjectPtr<UBehaviorTreeComponent>    BTComp;
	TWeakObjectPtr<UBlackboardComponent>      BBComp;

	TWeakObjectPtr<APawn> CachedTarget;

	static const FName TargetKeyName;
	static const FName PhaseExitKeyName;
};