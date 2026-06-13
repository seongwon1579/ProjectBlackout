#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BlackoutAIController.generated.h"

class UStateTreeAIComponent;

/**
 * Project Blackout AI 컨트롤러 베이스 클래스.
 * AI 캐릭터의 StateTree를 초기화하고 실행하는 역할을 담당.
 */
UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutAIController : public AAIController
{
	GENERATED_BODY()

public:
	ABlackoutAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	virtual void StartCombat() {}

protected:
	// virtual void OnPossess(APawn* InPawn) override;
	// virtual void OnUnPossess() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComp;
};
