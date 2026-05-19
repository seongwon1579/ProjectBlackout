#pragma once

#include "CoreMinimal.h"
#include "BlackoutInteractable.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/Actor.h"
#include "BlackoutAreaGate.generated.h"

class UBoxComponent;

/**
 * 쉘터↔보스 경계 게이트. 닫힘 = 물리 배리어. 직전 쉘터 페이즈에서 상호작용 시 Ready 토글.
 * 매치 상태가 TargetCombatPhase 가 되면 자동 Open, 벗어나면 자동 Close (전멸 후 재커밋 강제).
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
	virtual void BeginPlay() override;

	// 이 게이트가 여는 전투 페이즈 (MidBossCombat / MainBossCombat). 디자이너가 배치별 지정.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Gate")
	EBlackoutMatchState TargetCombatPhase = EBlackoutMatchState::MidBossCombat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Gate")
	TObjectPtr<UBoxComponent> Barrier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Gate")
	FText InteractionPrompt;

private:
	bool bIsOpen = false;

	UFUNCTION()
	void HandleMatchStateChanged(EBlackoutMatchState NewState);

	// TargetCombatPhase 직전 쉘터 페이즈 (MidBossCombat→ShelterPrep / MainBossCombat→ShelterMid).
	EBlackoutMatchState GetPrecedingShelterPhase() const;

	void ApplyOpenState(bool bOpen);
};
