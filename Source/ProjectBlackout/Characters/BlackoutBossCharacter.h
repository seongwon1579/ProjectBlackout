#pragma once

#include "CoreMinimal.h"
#include "BORavagerData.h"
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

	UFUNCTION()
	UBORavagerData* GetPatternData(FGameplayTag AbilityTag) const;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;


protected:
	virtual void BeginPlay() override;
	virtual void OnDeath() override;

	virtual void OnDamageReceived(const FOnAttributeChangeData& Data);
	
	APawn* ResolveInstigatorPawn(AActor* SourceActor) const;
	EBOBossPhase DetermineTargetPhase(float HealthRatio) const;
	
	void TryBindToHUD();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Data", meta = (Categories = "Ability"))
	TMap<FGameplayTag, TObjectPtr<UBORavagerData>> BossAbilityData;
	
};
