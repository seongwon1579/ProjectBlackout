#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutGameState.generated.h"

UCLASS()
class PROJECTBLACKOUT_API ABlackoutGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABlackoutGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 현재 매치 생애주기 상태. 서버에서 SetMatchState 로만 전환하고 클라에 리플리케이트.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentMatchState, Category = "Blackout|GameState")
	EBlackoutMatchState CurrentMatchState = EBlackoutMatchState::InLobby;

	// 서버 Authority 전용 세터. 같은 상태 중복 전환은 무시.
	UFUNCTION(BlueprintCallable, Category = "Blackout|GameState")
	void SetMatchState(EBlackoutMatchState NewState);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	float MatchTimer = 0.f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DestroyedPillarIds, Category = "Blackout|GameState")
	TArray<int32> DestroyedPillarIds;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	bool bRedMistPhaseActive = false;

protected:
	UFUNCTION()
	void OnRep_CurrentMatchState();

	UFUNCTION()
	void OnRep_DestroyedPillarIds();
};
