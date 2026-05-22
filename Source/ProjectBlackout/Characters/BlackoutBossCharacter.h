#pragma once

#include "CoreMinimal.h"
#include "BORavagerPatternData.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "GameplayEffectTypes.h"
#include "MotionWarpingComponent.h"
#include "Enum/BOBossPhase.h"

#include "BlackoutBossCharacter.generated.h"

class UBOBossData;
class UMotionWarpingComponent;

UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutBossCharacter : public ABlackoutEnemyCharacter
{
	GENERATED_BODY()

public:
	ABlackoutBossCharacter();

	//virtual void OnReturnToPool_Implementation() override;

	UFUNCTION()
	UBORavagerPatternData* GetPatternData(FGameplayTag AbilityTag) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

protected:
	virtual void BeginPlay() override;

	virtual void OnDeath() override;

	virtual void SetData();

	virtual void OnDamageReceived(const FOnAttributeChangeData& Data);
 
	virtual FText GetBossDisplayName() const;

	virtual EBOBossPhase DetermineTargetPhase(float HealthRatio);

	APawn* ResolveInstigatorPawn(AActor* SourceActor) const;

	void TryBindToHUD();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Data", meta = (Categories = "Ability"))
	TMap<FGameplayTag, TObjectPtr<UBORavagerPatternData>> BossPatternData;

	UPROPERTY(EditAnywhere, Category = "Blackout|Data")
	TObjectPtr<UBORavagerData> BossData;
};
