#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutPlayerState.generated.h"

class UBlackoutAmmoAttributeSet;
class UBlackoutPlayerAttributeSet;
class UBlackoutBaseAttributeSet;
class UBlackoutAbilitySystemComponent;
class UBOCharacterData;
class UBOConsumableData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBlackoutConsumableCountsChangedSignature, int32, BloodRootCount, int32, GulSerumCount);
DECLARE_MULTICAST_DELEGATE_OneParam(FBlackoutReadyStateChangedNativeSignature, bool);
DECLARE_MULTICAST_DELEGATE(FBlackoutPlayerNameChangedNativeSignature);
DECLARE_MULTICAST_DELEGATE(FBlackoutMatchStatsChangedNativeSignature);

UCLASS()
class PROJECTBLACKOUT_API ABlackoutPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ABlackoutPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Seamless Travel 시 새 PlayerState 로 보존 대상(SelectedClassTag) 복사.
	virtual void CopyProperties(APlayerState* NewPlayerState) override;

	UBlackoutAbilitySystemComponent* GetBlackoutAbilitySystemComponent() const { return AbilitySystemComponent; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState")
	void ApplyBattleTransitionPolicy(EBattleTransitionType TransitionType);

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState|Consumables")
	void SetConsumableCounts(int32 NewBloodRootCount, int32 NewGulSerumCount);

	void InitializeConsumablesFromCharacterData(const UBOCharacterData* CharacterData);

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|Consumables")
	int32 GetConsumableCount(FGameplayTag ConsumableTag) const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState|Consumables")
	bool SetConsumableCount(FGameplayTag ConsumableTag, int32 NewCount);

	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState|Consumables")
	bool ConsumeConsumable(FGameplayTag ConsumableTag, int32 Amount = 1);

	/** Ready Check 상태를 서버 권위로 갱신하고 로컬/원격에 동일한 변경 이벤트를 전달합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|PlayerState|Ready")
	void SetReadyState(bool bNewReady);

	void SetLoadedState(bool bNewLoaded);
	
	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|Ready")
	bool IsReady() const { return bIsReady; }
	
	bool IsLoaded() const { return bIsLoaded; }

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|State")
	bool IsDowned() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|State")
	bool IsReviving() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|State")
	bool IsBeingRevived() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|Cheat")
	bool HasInfiniteHealthCheat() const { return bInfiniteHealthCheat; }

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|Cheat")
	bool HasInfiniteStaminaCheat() const { return bInfiniteStaminaCheat; }

	UFUNCTION(BlueprintPure, Category = "Blackout|PlayerState|Cheat")
	bool HasInfiniteAmmoCheat() const { return bInfiniteAmmoCheat; }

	/** 개발용 치트 플래그를 갱신하고 필요한 어트리뷰트를 즉시 보정합니다. */
	void SetDebugCheatFlags(bool bNewInfiniteHealth, bool bNewInfiniteStamina, bool bNewInfiniteAmmo);

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Blackout|PlayerState")
	FGameplayTag SelectedClassTag;
	
	// 로그인 ID = 재접속 키
	// InitNewPlayer 에서 ?Acc= 파싱으로 설정
	FString AccountId;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_BloodRootCount, Category = "Blackout|PlayerState")
	int32 BloodRootCount = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_GulSerumCount, Category = "Blackout|PlayerState")
	int32 GulSerumCount = 0;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsReady, Category = "Blackout|PlayerState")
	bool bIsReady = false;
	
	bool bIsLoaded = false;


	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SurrenderVoteState, Category = "Blackout|PlayerState")
	bool bRequestedSurrender = false;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_SurrenderVoteState, Category = "Blackout|PlayerState")
	bool bVotedAgainstSurrender = false;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|PlayerState|Consumables")
	FBlackoutConsumableCountsChangedSignature OnConsumableCountsChanged;

	/** Ready Check 대기 애니메이션처럼 코드 전용 반응이 필요할 때 구독하는 네이티브 이벤트입니다. */
	FBlackoutReadyStateChangedNativeSignature OnReadyStateChangedNative;

	/** PlayerName 이 복제(변경)될 때 코드로 알림 — 파티 로스터가 닉네임 늦게 도착 시 갱신용. */
	FBlackoutPlayerNameChangedNativeSignature OnPlayerNameChangedNative;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MatchStats, Category = "Blackout|PlayerState|Stats")
	FBlackoutMatchStats MatchStats;

	/** 매치 통계가 갱신될 때 코드로 알림 — 스코어보드/요약 위젯 갱신용. */
	FBlackoutMatchStatsChangedNativeSignature OnMatchStatsChangedNative;

	// --- 집계 (기존 서버 경로에서 호출, 클라 RPC 금지) ---
	void AddDamageDealt(float Amount);
	void RecordKill(bool bWasMeleeKill);
	void RecordShotsFired(int32 Count = 1);
	void RecordShotsHit(int32 Count = 1);
	void RecordConsumableUsed();
	void RecordRevive();

protected:
	
	UFUNCTION()
	void OnRep_MatchStats();
	
	/** 기본 PlayerName 복제 콜백 — 늦게 도착하는 닉네임을 로스터에 알리려 오버라이드. */
	virtual void OnRep_PlayerName() override;

	UFUNCTION()
	void OnRep_IsReady();

	UFUNCTION()
	void OnRep_BloodRootCount();

	UFUNCTION()
	void OnRep_GulSerumCount();

	UFUNCTION()
	void OnRep_SurrenderVoteState();

	UFUNCTION()
	void OnRep_DebugCheatFlags();

	void BroadcastConsumableCounts();

	bool HasStateTag(FGameplayTag StateTag) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|GAS")
	TObjectPtr<UBlackoutAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<const UBlackoutBaseAttributeSet> BaseAttributeSet;

	UPROPERTY()
	TObjectPtr<const UBlackoutPlayerAttributeSet> PlayerAttributeSet;

	UPROPERTY()
	TObjectPtr<const UBlackoutAmmoAttributeSet> AmmoAttributeSet;
	
private:
	void BroadcastMatchStatsChanged();
	void BroadcastReadyStateChanged();
	void RestoreAtCheckpoint();
	void ApplyActiveCheatState();
	void ApplyInfiniteHealthCheat() const;
	void ApplyInfiniteStaminaCheat() const;
	void ApplyInfiniteAmmoCheat() const;

	UPROPERTY(ReplicatedUsing = OnRep_DebugCheatFlags)
	bool bInfiniteHealthCheat = false;

	UPROPERTY(ReplicatedUsing = OnRep_DebugCheatFlags)
	bool bInfiniteStaminaCheat = false;

	UPROPERTY(ReplicatedUsing = OnRep_DebugCheatFlags)
	bool bInfiniteAmmoCheat = false;

};
