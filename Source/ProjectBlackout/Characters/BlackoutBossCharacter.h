#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "GameplayEffectTypes.h"
#include "MotionWarpingComponent.h"
#include "BlackoutBossCharacter.generated.h"

class UBOBossData;
class UMotionWarpingComponent;
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
 * 수신된 피해를 기반으로 페이즈 전환을 관리.
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

protected:
	virtual void BeginPlay() override;

	/** 피해 수신 시 페이즈 전환 조건을 확인하기 위해 호출됨 */
	virtual void OnDamageReceived(UAbilitySystemComponent* Source,
	                              const FGameplayEffectSpec& Spec,
	                              FActiveGameplayEffectHandle Handle);

	/** 체력 컷라인 기준으로 페이즈 전환 여부를 평가 */
	virtual void EvaluatePhaseTransition();

	/** 자식 클래스에서 오버라이드하여 페이즈 전환 전용 로직 처리 */
	virtual void OnPhaseChanged(EBossPhase NewPhase);

	/** 페이즈 변경 이벤트를 브로드캐스트 */
	void BroadcastOnPhaseChanged();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	TObjectPtr<UBOBossData> BossData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	EBossPhase CurrentPhase;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss")
	int32 PhaseIndex;
};
