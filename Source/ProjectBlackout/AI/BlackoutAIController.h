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

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** StateTree가 참조하는 외부 데이터 핸들을 초기화 */
	virtual void InitStateTreeContext();

	/** AI Perception 초기화 (필요 시 서브클래스에서 오버라이드) */
	virtual void InitPerception();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComp;
};
