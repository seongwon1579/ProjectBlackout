// ─── 구현 내역 ───────────────────────
//  - 김민영: 처치 보상 드롭 테이블 데이터 에셋 구현 — 아이템 종류·가중치 기반 확률·보급 비율·소모품 지급 수량 구성
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/BlackoutDropItem.h"
#include "BORewardTableData.generated.h"

/**
 * 드롭 보상 설정 아이템 구조체
 */
USTRUCT(BlueprintType)
struct FBlackoutRewardItemConfig
{
	GENERATED_BODY()

	/** 드롭할 아이템 종류 (주무기 탄약 / 보조무기 탄약 / 소모품) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
	EBlackoutDropItemType ItemType = EBlackoutDropItemType::PrimaryAmmo;

	/** 드롭 확률 상대 가중치 (전체 가중치 합 대비 비율로 확률 계산) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ClampMin = "0.0"))
	float DropWeight = 1.0f;

	/** 보상 수치 (탄약일 경우 최대 예비 탄약 대비 보충 비율, 0.2f = 20% 충전 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SupplyRatio = 0.2f;

	/** 소모품일 경우 한 번에 지급할 개수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward", meta = (ClampMin = "1"))
	int32 ConsumableAmount = 1;
};

/**
 * 처치 보상 드롭 시스템에서 참조할 통합 드롭 테이블 데이터 에셋 클래스입니다.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBORewardTableData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 드롭 아이템 구성 및 설정 배열 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Reward")
	TArray<FBlackoutRewardItemConfig> RewardConfigs;
};
