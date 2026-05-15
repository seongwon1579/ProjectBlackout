#pragma once

#include "CoreMinimal.h"
#include "BORavagerData.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "GameplayEffectTypes.h"
#include "MotionWarpingComponent.h"
#include "BlackoutBossCharacter.generated.h"

class UBOBossData;
class UMotionWarpingComponent;
class UBOAggroComponent;
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
 * 보스 캐릭터(Shrewd, Ravager) 베이스 클래스.
 * 수신된 피해를 기반으로 페이즈 전환 및 어그로를 관리한다.
 */
UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutBossCharacter : public ABlackoutEnemyCharacter
{
	GENERATED_BODY()

public:
	ABlackoutBossCharacter();

	virtual void OnReturnToPool_Implementation() override;
	
	UFUNCTION()
	UBORavagerData* GetPatternData(FGameplayTag AbilityTag) const;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|Boss")
	FOnBossPhaseChangedSignature OnPhaseChangedDelegate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Aggro")
	TObjectPtr<UBOAggroComponent> AggroComponent;

protected:
	virtual void BeginPlay() override;
	virtual void OnDeath() override;

	virtual void OnDamageReceived(UAbilitySystemComponent* Source,
	                              const FGameplayEffectSpec& Spec,
	                              FActiveGameplayEffectHandle Handle);

	virtual void EvaluatePhaseTransition();
	virtual void OnPhaseChanged(EBossPhase NewPhase);
	void BroadcastOnPhaseChanged();
	
	void TryBindToHUD();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Data", meta = (Categories = "Ability"))
	TMap<FGameplayTag, TObjectPtr<UBORavagerData>> BossAbilityData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	EBossPhase CurrentPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	int32 PhaseIndex;
};
