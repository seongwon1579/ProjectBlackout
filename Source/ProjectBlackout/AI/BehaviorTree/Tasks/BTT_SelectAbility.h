// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 실행할 어빌리티 태그를 미리 골라 블랙보드에 저장하는 어빌리티 선택 Task
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SelectAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTT_SelectAbility : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_SelectAbility();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability")
	FGameplayTag AbilityTagToSet;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Output")
	FBlackboardKeySelector SelectedAbilityTagKey;
};
