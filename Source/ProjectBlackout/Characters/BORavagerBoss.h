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

public:
	ABORavagerBoss();


protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTree> ST_Ravager_Phases;

	/** Phase C 진입 시 공격 선/후딜 감소를 위한 애니메이션 배속 승수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Boss|Ravager")
	float AnimPlayRateMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss|Ravager")
	int32 SummonedMinionCount;
	
	virtual void BeginPlay() override;
};
