#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/IBossAggroProvider.h"
#include "BOAggroComponent.generated.h"

class APlayerState;
class UBOBossData;

UCLASS(ClassGroup = "Blackout|AI", meta = (BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBOAggroComponent : public UActorComponent, public IBossAggroProvider
{
	GENERATED_BODY()

public:
	UBOAggroComponent();

	virtual APawn* GetHighestAggroTarget() const override;
	virtual void   AddThreat(APawn* Source, float Amount) override;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Aggro")
	TObjectPtr<UBOBossData> BossData;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

private:
	TMap<TWeakObjectPtr<APlayerState>, float> DamageAccumulator;
	mutable TWeakObjectPtr<APawn> CurrentTarget;
	mutable float LastSwitchTime = 0.f;
};
