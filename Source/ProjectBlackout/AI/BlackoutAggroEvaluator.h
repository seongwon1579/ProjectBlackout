// Fill out your copyright notice in the Description page of Project Settings.

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

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAggroTargetChanged, APawn* /*NewTarget*/);

UCLASS()
class PROJECTBLACKOUT_API UBlackoutAggroEvaluator : public UObject
{
	GENERATED_BODY()

public:
	FOnAggroTargetChanged OnAggroTargetChanged;
	
	UBlackoutAggroEvaluator();

	virtual UWorld* GetWorld() const override;

	void Initialize(AAIController* InAIController, UAbilitySystemComponent* InASC);
	void Deinitialize();
	void RecordDamage(APawn* Source, float Amount);

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

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float DistanceWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float DPSWeight = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float LowHPWeight = 1.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float DPSWindowDuration = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Aggro", meta =(ClampMin = "0.0"))
	float MaxAggroRange = 10000.f;

	FGameplayTag TargetChangeTag;
	FDelegateHandle TargetChangeTagChangedHandle;

	float CalculateAggroScore(APawn* Target) const;
	void OnAggroTargetChangeTagChanged(const FGameplayTag Tag, int32 NewCount);
	void UpdateTarget();
	APawn* CalculateBestTarget(APawn* ExcludeTarget = nullptr) const;
};
