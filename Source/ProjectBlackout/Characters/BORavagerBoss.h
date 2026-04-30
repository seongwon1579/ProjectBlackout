#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutBossCharacter.h"
#include "AI/IBossAggroProvider.h"
#include "BORavagerBoss.generated.h"

class UStateTree;
class UBOAggroComponent;
class UGameplayAbility;

/**
 * Corrupted Ravager Boss (메인 보스)
 * 3-Phase 구조(Phase A/B/C)를 가지며, 체력 비율에 따라 전이.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORavagerBoss : public ABlackoutBossCharacter, public IBossAggroProvider
{
	GENERATED_BODY()

public:
	ABORavagerBoss();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Boss|Ravager")
	void EnterPhaseA();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Boss|Ravager")
	void EnterPhaseB();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Boss|Ravager")
	void EnterPhaseC();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Boss|Ravager")
	void SpawnMinionWave(int32 InPhaseIdx);
	
	// ── IBossAggroProvider ────────────────────────────────────────────────────
	virtual APawn* GetHighestAggroTarget() const override;
	virtual void   AddThreat(APawn* Source, float Amount) override;

	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Aggro")
	TObjectPtr<UBOAggroComponent> AggroComp;

protected:
	virtual void OnPhaseChanged(EBossPhase NewPhase) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTree> ST_Ravager_Phases;

	/** Phase C 진입 시 공격 선/후딜 감소를 위한 애니메이션 배속 승수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Boss|Ravager")
	float AnimPlayRateMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss|Ravager")
	int32 SummonedMinionCount;
	
	virtual void BeginPlay() override;
};
