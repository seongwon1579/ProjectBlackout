#pragma once

#include "CoreMinimal.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/Actor.h"
#include "BlackoutShelterZone.generated.h"

class UBoxComponent;
class APawn;
class UGameplayEffect;

/**
 * 쉘터 안전구역. 폰이 overlap 시 State.InShelter 부여 + 자원 보정 + 체크포인트 등록.
 * 이탈 시 State.InShelter 제거 + Effect.ShelterScoped 태그 GE 일괄 strip.
 * TargetShelterPhase 와 현재 매치 상태가 일치할 때만 효과 적용 (전투 페이즈 재진입 무효).
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutShelterZone : public AActor
{
	GENERATED_BODY()

public:
	ABlackoutShelterZone();

protected:
	virtual void BeginPlay() override;

	// 이 쉘터가 활성화되는 매치 페이즈 (ShelterPrep / ShelterMid). 디자이너가 배치별 지정.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Shelter")
	EBlackoutMatchState TargetShelterPhase = EBlackoutMatchState::ShelterPrep;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category="Blackout|Shelter")
	TSubclassOf<UGameplayEffect> ShelterStateEffectClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Shelter")
	TObjectPtr<UBoxComponent> Trigger;

	UFUNCTION()
	void OnTriggerBegin(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnTriggerEnd(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void HandleMatchStateChanged(EBlackoutMatchState NewState);

private:
	// 현재 존 안에 있는 폰들. 페이즈 전이 시 일괄 적용/strip 대상.
	UPROPERTY()
	TSet<TWeakObjectPtr<APawn>> OverlappingPawns;

	void ApplyShelterEffects(APawn* Pawn);
	void RemoveShelterEffects(APawn* Pawn);
	bool IsTargetPhaseActive() const;
	
	// binding 누락 spawn-time overlap 보정 (서버)
	void SnapshotCurrentOverlaps();
};
