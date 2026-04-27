#pragma once

#include "CoreMinimal.h"
#include "AI/FActionData.h"
#include "ActionPipeline.generated.h"

class IActionHandler;

// 파이프라인 단계별 결과 알림 (디버깅·어그로 갱신 훅으로 활용)
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActionStepExecuted, UObject* /*Handler*/, const FActionData&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnActionStepFailed,   UObject* /*Handler*/, const FActionData&);
DECLARE_MULTICAST_DELEGATE_OneParam (FOnActionChainCompleted, const FActionData&);

/**
 * GA 실행 전 파이프라인을 순서대로 실행한다.
 *
 * 역할: "누구에게, 실행 가능한가" 판단 (타겟 결정 + 유효성 검사)
 * 비역할: GA 실행 (BTTask 담당), 몽타주·데미지·이펙트 (GA 담당)
 *
 * 핸들러 추가: RegisterHandler() 한 줄 + 핸들러 클래스만 추가하면 된다.
 * 실행 순서: IActionHandler::GetPriority() 오름차순.
 */
UCLASS()
class PROJECTBLACKOUT_API UActionPipeline : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();

	/** 파이프라인에 핸들러를 추가한다. 실행 순서는 GetPriority()로 결정. */
	void RegisterHandler(UObject* Handler);

	/**
	 * 파이프라인을 순서대로 실행한다.
	 * @return true  — 모든 핸들러 통과 (GA 실행 진행 가능)
	 * @return false — 중간 실패, 이미 실행된 핸들러 Undo 완료
	 */
	bool Execute(FActionData& Data);

	FOnActionStepExecuted   OnActionStepExecuted;
	FOnActionStepFailed     OnActionStepFailed;
	FOnActionChainCompleted OnActionChainCompleted;

private:
	UPROPERTY()
	TArray<TObjectPtr<UObject>> HandlerList;

	TArray<IActionHandler*> GetSortedHandlers() const;
	void RollbackExecutedHandlers(const TArray<IActionHandler*>& Executed, const FActionData& Data);
};
