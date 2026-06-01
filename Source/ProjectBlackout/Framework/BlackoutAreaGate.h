#pragma once

#include "CoreMinimal.h"
#include "BlackoutInteractable.h"
#include "GameFramework/Actor.h"
#include "BlackoutAreaGate.generated.h"

class UBoxComponent;

/**
 * 이동 비석. ShelterPrep 에서 상호작용 시 Ready 토글 → 전원 Ready 시 StartBattle(이동).
 * (구 쉘터↔보스 경계 게이트에서 물리 배리어/Open 제거 — seamless 맵 분리로 불필요)
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutAreaGate : public AActor, public IBlackoutInteractable
{
	GENERATED_BODY()

public:
	ABlackoutAreaGate();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:

	UPROPERTY(VisibleAnywhere , BlueprintReadOnly , Category = "Blackout|Gate")
	TObjectPtr<UBoxComponent> Barrier;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category = "Blackout|Gate")
	FText InteractionPrompt;
	
};
