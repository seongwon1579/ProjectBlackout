#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "Templates/SubclassOf.h"
#include "BORootWraith.generated.h"

class UStateTree;
class ABOProjectile;

/**
 * Root Wraith 엘리트 미니언.
 * 순수 StateTree를 이용해 원거리 2연사 및 시야 밖 점멸을 수행.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORootWraith : public ABlackoutEnemyCharacter
{
	GENERATED_BODY()

public:
	ABORootWraith();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Minion")
	void FireTwinArrows();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Minion")
	void TeleportOutOfSight();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTree> ST_RootWraith;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> ArrowProjectileClass;
};
