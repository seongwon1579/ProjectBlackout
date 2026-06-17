// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 회피 방향에 맞춰 좌/우 회피 어빌리티 태그를 선택하는 회피 실행 Task (ActivateAbility 상속)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "AI/BehaviorTree/Tasks/BTT_ActivateAbility.h"
#include "BTT_ActivateEvadeAbility.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBTT_ActivateEvadeAbility : public UBTT_ActivateAbility
{
	GENERATED_BODY()

public:
	UBTT_ActivateEvadeAbility();
	
protected:
	virtual FGameplayTag ResolveAbilityTag(UBehaviorTreeComponent& OwnerComp) const override;
	virtual FString GetStaticDescription() const override;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Ability", meta = (Categories = "Ability"))
	FGameplayTag LeftAbilityTag;

	UPROPERTY(EditAnywhere, Category = "Blackout|Ability", meta = (Categories = "Ability"))
	FGameplayTag RightAbilityTag;

	UPROPERTY(EditAnywhere, Category = "Blackout|Blackboard|Input")
	FBlackboardKeySelector EvadeDirectionKey;
};
