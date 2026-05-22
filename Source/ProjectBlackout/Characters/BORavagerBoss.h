#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutBossCharacter.h"
#include "BORavagerBoss.generated.h"

class UStateTree;
class UBOAggroComponent;
class UGameplayAbility;
class USphereComponent;

/**
 * Corrupted Ravager Boss (메인 보스)
 * 3-Phase 구조(Phase A/B/C)를 가지며, 체력 비율에 따라 전이.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORavagerBoss : public ABlackoutBossCharacter
{
	GENERATED_BODY()

// public:
// 	virtual UDataAsset* GetPatternData(FGameplayTag AbilityTag) const;
// 	
// protected:
// 	virtual void SetData() override;
// 	virtual EBOBossPhase DetermineTargetPhase(float HealthRatio) const override;
// 	virtual FText GetBossDisplayName() const override;
//
// 	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
// 	TMap<FGameplayTag, TObjectPtr<UBORavagerPatternData>> BossPatternData;
// 	
// 	UPROPERTY(EditAnywhere, Category = "Blackout|Data")
// 	TObjectPtr<UBORavagerData> BossData;
};
