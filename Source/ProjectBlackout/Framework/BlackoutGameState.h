#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "BlackoutGameState.generated.h"

UCLASS()
class PROJECTBLACKOUT_API ABlackoutGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABlackoutGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	float MatchTimer = 0.f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DestroyedPillarIds, Category = "Blackout|GameState")
	TArray<int32> DestroyedPillarIds;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	bool bRedMistPhaseActive = false;

protected:
	UFUNCTION()
	void OnRep_DestroyedPillarIds();
};
