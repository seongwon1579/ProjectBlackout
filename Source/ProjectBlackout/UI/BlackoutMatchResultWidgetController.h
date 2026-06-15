#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UI/BlackoutMatchResultTypes.h"
#include "BlackoutMatchResultWidgetController.generated.h"

class ABlackoutGameState;
class ABlackoutPlayerState;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBlackoutMatchResultVisibilityChangedSignature,
	bool, bIsVisible);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FBlackoutMatchResultRebuiltSignature,
	const FBlackoutMatchResultSummaryData&, SummaryData,
	const TArray<FBlackoutMatchResultPlayerStatsData>&, PlayerStatsList);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBlackoutMatchResultPlayerStatsChangedSignature,
	const FBlackoutMatchResultPlayerStatsData&, PlayerStatsData);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBlackoutMatchResultLocalConfirmStateChangedSignature,
	bool, bHasConfirmed);

struct FBlackoutMatchResultPlayerBinding
{
	TWeakObjectPtr<ABlackoutPlayerState> PlayerState;
	FDelegateHandle MatchStatsChangedHandle;
	FDelegateHandle NameChangedHandle;
};

UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBlackoutMatchResultWidgetController : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	bool Initialize(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void BindCallbacksToDependencies();

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void BroadcastInitialResult();

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void RefreshResult();

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Result")
	void RequestConfirmResult();

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Result")
	FBlackoutMatchResultVisibilityChangedSignature OnResultVisibilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Result")
	FBlackoutMatchResultRebuiltSignature OnResultRebuilt;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Result")
	FBlackoutMatchResultPlayerStatsChangedSignature OnPlayerStatsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Result")
	FBlackoutMatchResultLocalConfirmStateChangedSignature OnLocalConfirmStateChanged;

private:
	bool ResolveDependencies(APlayerController* InPlayerController);
	void BindPlayer(ABlackoutPlayerState* PlayerState);
	void UnbindPlayer(ABlackoutPlayerState* PlayerState);
	void UnbindAllPlayers();
	void BroadcastPlayerStats(ABlackoutPlayerState* PlayerState);
	FBlackoutMatchResultSummaryData BuildSummaryData(const TArray<FBlackoutMatchResultPlayerStatsData>& PlayerStatsList) const;
	FBlackoutMatchResultPlayerStatsData BuildPlayerStatsData(ABlackoutPlayerState* PlayerState, int32 DisplayOrder) const;
	TArray<ABlackoutPlayerState*> GetParticipantPlayerStates() const;
	bool IsResultVisible() const;

	UFUNCTION()
	void HandlePlayerArrayChanged();

	UFUNCTION()
	void HandleMatchStateChanged(EBlackoutMatchState NewState);

	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<ABlackoutGameState> GameState;
	TWeakObjectPtr<ABlackoutPlayerState> LocalPlayerState;
	TMap<ABlackoutPlayerState*, FBlackoutMatchResultPlayerBinding> PlayerBindings;
	TSet<TObjectKey<ABlackoutPlayerState>> ConfirmedPlayerStates;
	bool bGameStateCallbacksBound = false;
	bool bLocalPlayerConfirmed = false;
};
