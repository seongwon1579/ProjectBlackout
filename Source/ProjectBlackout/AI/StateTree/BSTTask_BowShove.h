#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_BowShove.generated.h"

class ABORootWraith;
class APawn;

/**
 * Root Wraith 근접 밀치기 StateTree Task.
 * 플레이어가 MeleeDetectRadius 이내로 진입했을 때 활대를 휘두르며 강하게 밀쳐내고,
 * 이후 후퇴 거리(PostShoveBackstepDistance)만큼 다시 원거리로 복귀한다.
 *
 * 흐름:
 *   EnterState → OwnerCharacter->BowShove(TargetPawn) 호출 (임펄스 적용)
 *   Tick       → 후퇴 이동 완료 여부 확인 → Succeeded 반환
 */
USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_BowShoveInstanceData
{
	GENERATED_BODY()

	/** 밀치기를 수행할 Root Wraith 캐릭터. */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ABORootWraith> OwnerCharacter;

	/** 밀칠 대상 폰 (플레이어). */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> TargetPawn;

	/** 밀치기 임펄스 강도 (cm/s). */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float ShoveImpulse = 1200.f;

	/** 밀치기 유효 사거리 (cm). 이 범위 밖이면 즉시 Failed. */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float ShoveRange = 180.f;

	/** 밀치기 후 Wraith 가 자동으로 후퇴할 거리 (cm). */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float PostShoveBackstepDistance = 600.f;
};

USTRUCT(meta = (DisplayName = "Bow Shove", Category = "Blackout|Minion"))
struct PROJECTBLACKOUT_API FBSTTask_BowShove : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_BowShoveInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/**
	 * 사거리 확인 후 BowShove() 호출.
	 * 범위 초과 시 Failed, 정상 진입 시 Running(후퇴 모션 대기).
	 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** 후퇴 이동 완료 여부를 매 틱 확인. 완료 시 Succeeded 반환. */
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
