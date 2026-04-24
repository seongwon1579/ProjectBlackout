#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "BOAICalcHelper.generated.h"

UENUM(BlueprintType)
enum class EBOTurnDirection : uint8
{
	None  UMETA(DisplayName = "No Turn"),
	Left  UMETA(DisplayName = "Turn Left"),
	Right UMETA(DisplayName = "Turn Right"),
};

/**
 * AI 공간 계산(거리·회전 방향) 정적 헬퍼.
 * 어빌리티 사거리 체크, BT 노드 회전 판단 등 반복 계산을 중앙화한다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAICalcHelper : public UObject
{
	GENERATED_BODY()

public:
	// ── 거리 ─────────────────────────────────────────────────────────────────

	/** Self → Target 2D 거리가 Range 이내이면 true. sqrt 없이 DistSquared로 비교. */
	static bool IsWithinRange(const AActor* Self, const AActor* Target, float Range);

	/** 두 액터 간 2D 거리(cm)를 반환. */
	static float GetDistance2D(const AActor* A, const AActor* B);

	// ── 회전 방향 ─────────────────────────────────────────────────────────────

	/**
	 * Self 가 Target 을 정면으로 바라보기 위해 어느 쪽으로 회전해야 하는지 계산.
	 *
	 * @param Threshold      이 각도(도) 미만이면 None 반환.
	 * @param OutAngleDelta  부호 있는 회전각(도). 양수 = 오른쪽, 음수 = 왼쪽.
	 */
	static EBOTurnDirection GetTurnDirection(const AActor* Self, const AActor* Target,
	                                          float Threshold, float& OutAngleDelta);
};