// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 거리/DPS/저체력 가중 점수로 어그로 타겟을 산정하는 평가기 (ST·BT 공용, 사망/다운 타겟 제외)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "BlackoutAggroEvaluator.generated.h"

/**
 * 
 */
class AAIController;
class UAbilitySystemComponent;
class AGameModeBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAggroTargetChanged, APawn* /*NewTarget*/);

UCLASS(EditInlineNew, DefaultToInstanced)
class PROJECTBLACKOUT_API UBlackoutAggroEvaluator : public UObject
{
	GENERATED_BODY()

public:
	FOnAggroTargetChanged OnAggroTargetChanged;
	
	virtual UWorld* GetWorld() const override;
	void Initialize(AAIController* InAIController, UAbilitySystemComponent* InASC);
	void Deinitialize();
	void RecordDamage(APawn* Source, float Amount);
	void StartAggroEvaluation();

private:
	struct FDamageRecord
	{
		float Timestamp = 0.f;
		float Amount = 0.f;
	};

	struct FPlayerCombatData
	{
		TArray<FDamageRecord> DamageRecords;

		float GetDamageInWindow(float CurrentTime, float WindowDuration)
		{
			// 데미지를 준 시점과 현재시점에서 WindowDuration 이상 지난 오래된 데이터는 Remove
			DamageRecords.RemoveAll([=](const FDamageRecord& Record)
			{
				return CurrentTime - Record.Timestamp > WindowDuration;
			});
			float TotalDamage = 0.f;

			for (const FDamageRecord& Record : DamageRecords)
			{
				TotalDamage += Record.Amount;
			}
			return TotalDamage;
		}
	};

	mutable TMap<TWeakObjectPtr<APawn>, FPlayerCombatData> CombatDataMap;

	UPROPERTY(Transient)
	TObjectPtr<AAIController> CachedOwnerAIController;

	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	UPROPERTY(EditAnywhere, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float DistanceWeight = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float DPSWeight = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float LowHPWeight = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float DPSWindowDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float MaxAggroRange = 10000.f;
	
	void OnAggroTargetChangeTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	APawn* CalculateBestTarget(APawn* ExcludeTarget = nullptr) const;
	float CalculateAggroScore(APawn* Target) const;
	
	void UpdateTarget();
	
	void RegisterTagEvents();
	void UnregisterTagEvents();
	
	UPROPERTY()
	TWeakObjectPtr<APawn> CurrentTarget;
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> CurrentTargetASC;
	
	FDelegateHandle TargetChangeTagChangedHandle;
	FGameplayTag TargetChangeTag;
	
	FDelegateHandle TargetDownTagHandle;
	FGameplayTag DownTag;
	
	void WatchTargetDownState(APawn* Target);
	void UnWatchTargetDownState();
	void OnTargetDownTagChanged(const FGameplayTag Tag, int32 NewCount);
	bool IsTargetInvalid(APawn* Target) const;
};
