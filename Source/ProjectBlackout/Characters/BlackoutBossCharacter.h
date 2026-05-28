#pragma once

#include "CoreMinimal.h"
#include "BORavagerPatternData.h"
#include "BORavagerStatData.h"
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

protected:
	virtual void BeginPlay() override;
	
	virtual void SetData() {}

	virtual void OnDamageReceived(const FOnAttributeChangeData& Data) {}
 
	virtual FText GetBossDisplayName() const { return FText::FromString(TEXT("")); }
	
	void TryBindToHUD();
};
