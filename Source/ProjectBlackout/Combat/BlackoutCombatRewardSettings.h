#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayEffect.h"
#include "BlackoutCombatRewardSettings.generated.h"

/**
 * 전투 보상 시스템의 전역 설정입니다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Blackout Combat Reward Settings"))
class PROJECTBLACKOUT_API UBlackoutCombatRewardSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UBlackoutCombatRewardSettings* Get();

	virtual FName GetCategoryName() const override { return TEXT("Blackout"); }

	/** 사망 확정 직후 적용할 보상 GameplayEffect입니다. 이 GE의 Execution에 BP ExecCalc_CombatReward를 지정합니다. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Blackout|Reward")
	TSoftClassPtr<UGameplayEffect> CombatRewardEffectClass;
};
