#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutGameState.generated.h"

class UBOCharacterRoster;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBlackoutPlayerArrayChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlackoutMatchStateChangedSignature, EBlackoutMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBlackoutSurrenderVoteStateChangedSignature, bool, bIsActive, int32, YesCount, int32, NoCount, float, EndTimeSeconds);

UCLASS()
class PROJECTBLACKOUT_API ABlackoutGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ABlackoutGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|GameState")
	FBlackoutPlayerArrayChangedSignature OnPlayerArrayChanged;

	// 매치 상태 변경 통지 (서버 SetMatchState + 클라 OnRep 양쪽 broadcast). 게이트/HUD/보스 활성이 구독.
	UPROPERTY(BlueprintAssignable, Category = "Blackout|GameState")
	FBlackoutMatchStateChangedSignature OnMatchStateChanged;

	// 현재 매치 생애주기 상태. 서버에서 SetMatchState 로만 전환하고 클라에 리플리케이트.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentMatchState, Category = "Blackout|GameState")
	EBlackoutMatchState CurrentMatchState = EBlackoutMatchState::WaitingForPlayers;

	// 서버 Authority 전용 세터. 같은 상태 중복 전환은 무시.
	UFUNCTION(BlueprintCallable, Category = "Blackout|GameState")
	void SetMatchState(EBlackoutMatchState NewState);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	float MatchTimer = 0.f;
	
	/** 선택 가능 캐릭터 목록  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Blackout|GameState")
	TObjectPtr<UBOCharacterRoster> CharacterRoster;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DestroyedPillarIds, Category = "Blackout|GameState")
	TArray<int32> DestroyedPillarIds;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	bool bRedMistPhaseActive = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SurrenderVoteActive, Category = "Blackout|GameState")
	bool bIsSurrenderVoteActive = false;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	int32 SurrenderVoteYesCount = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	int32 SurrenderVoteNoCount = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	int32 RequiredSurrenderVoteCount = 0;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	float SurrenderVoteEndTimeSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|GameState")
	float SurrenderVoteCooldownEndTime = 0.f;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|GameState")
	FBlackoutSurrenderVoteStateChangedSignature OnSurrenderVoteStateChanged;

protected:
	UFUNCTION()
	void OnRep_CurrentMatchState();

	UFUNCTION()
	void OnRep_DestroyedPillarIds();

	UFUNCTION()
	void OnRep_SurrenderVoteActive();
};
