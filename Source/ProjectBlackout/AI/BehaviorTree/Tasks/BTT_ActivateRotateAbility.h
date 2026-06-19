// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 현재 부호각을 이벤트 데이터로 실어 회전 어빌리티를 실행하는 Task (ActivateAbility 상속)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "AI/BehaviorTree/Tasks/BTT_ActivateAbility.h"

#include "BTT_ActivateRotateAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTT_ActivateRotateAbility : public UBTT_ActivateAbility
{
	GENERATED_BODY()
	
public:
	UBTT_ActivateRotateAbility();
	
protected:
	virtual void PrepareEventData(FGameplayEventData& EventData, UBlackboardComponent* BB) override;
	
	virtual FString GetStaticDescription() const override;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector SignedAngleKey;

};
