#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BlackoutInteractable.h"
#include "BlackoutConsumablePickup.generated.h"

class ABlackoutPlayerState;
class UBOConsumableData;
class UStaticMeshComponent;
class USphereComponent;

/**
 * 드랍된 소모품을 [E] 상호작용으로 획득하는 1차 픽업 액터입니다.
 * 현재는 BloodRoot / GulSerum 계열처럼 PlayerState 수량으로 관리되는 소모품만 지원합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABlackoutConsumablePickup : public AActor, public IBlackoutInteractable
{
	GENERATED_BODY()

public:
	ABlackoutConsumablePickup();

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Pickup")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Pickup")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	/** 이 픽업이 지급할 소모품 데이터입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pickup")
	TObjectPtr<UBOConsumableData> ConsumableData;

	/** 한 번 획득할 때 증가할 소모품 수량입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pickup", meta = (ClampMin = 1))
	int32 PickupAmount = 1;

	/** 획득 성공 시 액터를 즉시 제거할지 여부입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pickup")
	bool bDestroyOnPickup = true;

private:
	bool TryResolvePickupState(
		AActor* Interactor,
		ABlackoutPlayerState*& OutPlayerState,
		int32& OutCurrentCount,
		int32& OutMaxCount) const;

	/** 동일 프레임 중복 획득을 막기 위한 서버 전용 플래그입니다. */
	UPROPERTY(Transient)
	bool bIsCollected = false;
};
