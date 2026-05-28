#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/BlackoutMinionCharacter.h"
#include "Templates/SubclassOf.h"
#include "BORootWraith.generated.h"

class UStateTree;
class ABOProjectile;

/**
 * Root Wraith 엘리트 미니언.
 * 순수 StateTree를 이용해 원거리 2연사 및 시야 밖 점멸을 수행.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORootWraith : public ABlackoutMinionCharacter
{
	GENERATED_BODY()

public:
	ABORootWraith();

	virtual void BeginPlay() override;
	virtual void OnSpawnFromPool_Implementation() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Visual")
	FGameplayTag SpawnGameplayCueTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTree> ST_RootWraith;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> ArrowProjectileClass;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Blackout|Combat")
	float MeleeDetectRadius= 300.0f;
	
	/** 이동속도 */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Blackout|Movement" ,meta=(ClampMin=0.0f , UIMin=0.0f , UIMax=300.0f))
	float DefaultFlySpeed = 75.f;

private:
	void PlayTeleportEndGameplayCue();
	
};
