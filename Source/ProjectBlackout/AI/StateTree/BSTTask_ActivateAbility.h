// ─── 구현 내역 ───────────────────────
//  - 조성원: 태그로 어빌리티를 실행하고 부여 대기(타임아웃)·실행 핸들 추적까지 처리하는 StateTree Task
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "BSTTask_ActivateAbility.generated.h"

class APawn;
class AActor;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_ActivateAbilityInstanceData
{
	GENERATED_BODY()

	/** 어빌리티를 실행할 대상 폰 */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> OwnerPawn;

	/** ⭐ 이벨류에이터 등으로부터 바인딩 받아올 타겟 액터 (EventData.Target에 주입됨) */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<AActor> TargetActor;

	/** 실행할 어빌리티의 Gameplay Tag (이벤트 태그) */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag AbilityTag;

	/** 어빌리티가 부여될 때까지 기다려줄 최대 시간 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float ActivateTimeout = 2.0f;

	/** 실행된 어빌리티를 추적하기 위한 핸들 */
	UPROPERTY()
	FGameplayAbilitySpecHandle ActiveHandle;

	/** 대기한 시간 누적용 */
	UPROPERTY()
	float WaitElapsed = 0.0f;
};

USTRUCT(meta = (DisplayName = "Activate Ability", Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_ActivateAbility : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_ActivateAbilityInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};