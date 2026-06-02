// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "BlackoutAggroComponent.generated.h"

class APawn;
class UAbilitySystemComponent;

// 어그로 타겟 변경 통지 (서버에서만)
DECLARE_MULTICAST_DELEGATE_OneParam(FBlackoutAggroTargetChanged , APawn* /*NewTarget*/);

UCLASS(ClassGroup=(Blackout), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutAggroComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBlackoutAggroComponent();
	
	// StateTree/ BT 구독 , 결정은 서버
	FBlackoutAggroTargetChanged OnTargetChanged;
	
	// 현재 어그로 타겟 (없으면 nullptr) 
	APawn* GetCurrentTarget() const {return CurrentTarget;}
	
	// 데미지 누적 기록 ( 소유 Pawn OnDamageReceived 에서 호출 )
	void RecordDamage(APawn* Source , float Amount);
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// -- 스코어 가중치 --
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Aggro" , meta=(ClampMin="0.0"))
	float DistanceWeight  = 1.0f;
	
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Aggro" , meta=(ClampMin="0.0"))
	float DPSWeight = 2.0f;
	
	UPROPERTY(EditDefaultsOnly ,  Category="Blackout|Aggro" , meta=(ClampMin="0.0"))
	float LowHPWeight = 1.5f; 
	
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Aggro" , meta=(ClampMin="0.1"))
	float DPSWindowDuration = 3.0f;
	
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Aggro" , meta=(ClampMin="0.0"))
	float MaxAggroRange = 10000.0f;
	
	// 재평가 주기 (초)
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Aggro" , meta=(ClampMin="0.05"))
	float ReEvalInterval = 0.3f;
	
	// 새 타겟이 현재보다 비율 이상 높아야 교체 (0.2 = 20%)
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Aggro" , meta=(ClampMin="0.0"))
	float SwitchScoreMargin  = 0.2f;
	
	// 교체 후 최소 유지 시간 (초)
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Aggro" , meta=(ClampMin="0.0"))
	float MinDwellTime = 1.5f;
	
private:
	struct FDamageRecord
	{
		float Timestamp =0.0f;
		float Amount =0.0f;
	};
	
	
	// 타겟 별 데미지 누적 
	TMap<TWeakObjectPtr<APawn> , TArray<FDamageRecord> >  DamageRecords;
	
	UPROPERTY(Transient)
	TObjectPtr<APawn> CurrentTarget;
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilitySystemComponent> OwnerASC;
	
	float LastSwitchTime  =0.0f;
	FTimerHandle ReEvalTimerHandle;
	FGameplayTag TargetChangeTag;
	FDelegateHandle TargetChangeTagHandle;
	
	// 주기 재평가 진입
	void ReEvaluate();
	
	// 필터 통과 후보중 최고 점수 타겟 + 점수 
	APawn* CalculateBestTarget(float& OutBestScore) const;
	
	// 단일 타겟 점수 ( 거리 + DPS + 저체력 )
	float CalculateScore(APawn* Target) const;
	
	// 후보 유효성 검사 ( 캐릭터 + 생존 + 사거리 )
	bool IsValidTarget(APawn* Target) const;
	
	// 회피 태크 복귀시 즉시 평가
	void OnTargetChangeTagChanged(const FGameplayTag Tag , int32 NewCount);
	
};
