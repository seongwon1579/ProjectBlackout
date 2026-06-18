// ─── 구현 내역 ───────────────────────
//  - 김민영: 텔레포트(Teleport) StateTree Task 정의
//  - 최승현: 시야 밖 위치/고도 변동 + 쿨다운 기반 점멸 이동 실제 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "BSTTask_Teleport.generated.h"

class APawn;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_TeleportInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<APawn> OwnerPawn;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float DesiredOffsetRange = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bRequireOutOfSight = true;
};

USTRUCT(meta = (DisplayName = "Teleport", Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_Teleport : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_TeleportInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
