#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "BORootHollow.generated.h"

class UStateTree;

/**
 * Root Hollow Minion
 * 일반 미니언 — 순수 StateTree를 이용해 행동(돌진 등)을 수행.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORootHollow : public ABlackoutEnemyCharacter
{
	GENERATED_BODY()

public:
	ABORootHollow();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Minion")
	void PerformCharge();

protected:
	virtual void OnDeath() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTree> ST_RootHollow;
};
