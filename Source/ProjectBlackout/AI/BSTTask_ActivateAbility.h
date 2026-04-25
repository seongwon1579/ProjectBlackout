#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "BSTTask_ActivateAbility.generated.h"

class UAbilitySystemComponent;

USTRUCT()
struct PROJECTBLACKOUT_API FBSTTask_ActivateAbilityInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bWaitForEnd = true;
};

USTRUCT(meta = (DisplayName = "Activate Ability", Category = "Blackout|AI"))
struct PROJECTBLACKOUT_API FBSTTask_ActivateAbility : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FBSTTask_ActivateAbilityInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
