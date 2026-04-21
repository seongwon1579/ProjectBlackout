#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "BlackoutBossCharacter.generated.h"

class UBOBossData;
struct FGameplayEffectSpec;

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	None        UMETA(DisplayName = "None"),
	PhaseA      UMETA(DisplayName = "Phase A"),
	PhaseB      UMETA(DisplayName = "Phase B"),
	PhaseC      UMETA(DisplayName = "Phase C"),
	Platform    UMETA(DisplayName = "Platform Phase (Shrewd)"),
	Ground      UMETA(DisplayName = "Ground Phase (Shrewd)")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChangedSignature, EBossPhase, NewPhase);

/**
 * Base class for Boss Characters (Shrewd, Ravager).
 * Manages Phase Transitions based on damage received.
 */
UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutBossCharacter : public ABlackoutEnemyCharacter
{
	GENERATED_BODY()

public:
	ABlackoutBossCharacter();

	virtual void OnReturnToPool_Implementation() override;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Boss")
	FOnBossPhaseChangedSignature OnPhaseChangedDelegate;

protected:
	virtual void BeginPlay() override;

	/** Called when damage is received to check for phase transitions */
	virtual void OnDamageReceived(const FGameplayEffectSpec& Spec);

	/** Evaluates if a phase transition should occur based on health cutlines */
	virtual void EvaluatePhaseTransition();

	/** Overridden by child classes to handle specific phase change logic */
	virtual void OnPhaseChanged(EBossPhase NewPhase);

	/** Broadcasts the phase change event */
	void BroadcastOnPhaseChanged();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	TObjectPtr<UBOBossData> BossData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	EBossPhase CurrentPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	int32 PhaseIndex;
};
