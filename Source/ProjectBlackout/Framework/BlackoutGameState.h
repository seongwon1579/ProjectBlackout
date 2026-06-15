#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutGameState.generated.h"

class UBOCharacterRoster;
class ABlackoutPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBlackoutPlayerArrayChangedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlackoutMatchStateChangedSignature, EBlackoutMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FBlackoutSurrenderVoteStateChangedSignature, bool, bIsActive, int32, YesCount, int32, NoCount, float, EndTimeSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBlackoutMatchResultStateChangedSignature);

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

	UPROPERTY(BlueprintAssignable, Category = "Blackout|GameState|Result")
	FBlackoutMatchResultStateChangedSignature OnMatchResultStateChanged;

	// 결과창 표시 여부. 보스 처치 후 서버 타이머가 켜고 이동 직전에 끕니다.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchResultState, Category = "Blackout|GameState|Result")
	bool bIsMatchResultVisible = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchResultState, Category = "Blackout|GameState|Result")
	EBossType DefeatedBossType = EBossType::Mid;

	// 결과창이 표시된 서버 월드 시간. 클라이언트 카운트다운 보정용입니다.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchResultState, Category = "Blackout|GameState|Result")
	float MatchResultVisibleServerTime = 0.f;

	// 자동 이동이 예약된 서버 월드 시간. UI에서 남은 시간을 계산할 수 있습니다.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchResultState, Category = "Blackout|GameState|Result")
	float MatchResultAutoTravelServerTime = 0.f;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchResultState, Category = "Blackout|GameState|Result")
	TArray<TObjectPtr<ABlackoutPlayerState>> MatchResultParticipants;

	UFUNCTION(BlueprintCallable, Category = "Blackout|GameState|Result")
	void SetMatchResultState(
		bool bNewVisible,
		EBossType NewDefeatedBossType,
		float VisibleServerTime,
		float AutoTravelServerTime,
		const TArray<ABlackoutPlayerState*>& Participants);

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

	UFUNCTION()
	void OnRep_MatchResultState();

private:
	// ShelterPrep 진입 시 로컬 PC 의 클래스 선택 UI 를 연다 (서버/클라 공통, 로컬 1개만).
	void TryOpenLocalClassSelectUI();
};
