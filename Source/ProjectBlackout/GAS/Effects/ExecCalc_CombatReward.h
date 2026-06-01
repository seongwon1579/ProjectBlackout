#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_CombatReward.generated.h"

class UAbilitySystemComponent;

/**
 * 전투 보상 실행 계산기 (TDD v5 §5.1)
 * 클래스 및 킬 조건에 따라 월드 드롭 아이템을 스폰합니다.
 */
UCLASS(Blueprintable, BlueprintType)
class PROJECTBLACKOUT_API UExecCalc_CombatReward : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_CombatReward();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	/** 서버 사망 확정 직후 마지막 타격 Spec을 기준으로 보상 GE를 타겟에게 적용합니다. */
	static bool ApplyConfiguredRewardEffect(
		const FGameplayEffectSpecHandle& DamageSpecHandle,
		UAbilitySystemComponent* TargetASC);

protected:
	/** 보상 확률 및 보급량 설정을 정의한 데이터 에셋 테이블입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Reward")
	TObjectPtr<class UBORewardTableData> RewardTable;

	/** 스폰할 통합 드롭 아이템 블루프린트 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Reward")
	TSubclassOf<class ABlackoutDropItem> DropItemClass;

	/** 사망 위치 주변 드롭 산포 반경입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Reward|Placement", meta = (ClampMin = "0.0"))
	float DropScatterRadius = 60.0f;

	/** 바닥 탐색 라인트레이스 시작점의 상단 거리입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Reward|Placement", meta = (ClampMin = "0.0"))
	float DropGroundTraceUpDistance = 300.0f;

	/** 바닥 탐색 라인트레이스 하단 거리입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Reward|Placement", meta = (ClampMin = "0.0"))
	float DropGroundTraceDownDistance = 1500.0f;

	/** 바닥 ImpactPoint에서 드롭 아이템을 살짝 띄울 높이입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Reward|Placement", meta = (ClampMin = "0.0"))
	float DropGroundOffset = 5.0f;

private:
	static bool TrySpawnRewardDropInternal(
		const FGameplayEffectSpec& RewardSpec,
		UAbilitySystemComponent* SourceASC,
		UAbilitySystemComponent* TargetASC,
		TSubclassOf<class ABlackoutDropItem> InDropItemClass,
		float InDropScatterRadius,
		float InDropGroundTraceUpDistance,
		float InDropGroundTraceDownDistance,
		float InDropGroundOffset,
		class UBORewardTableData* InRewardTable);
};
