#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutBossCharacter.h"
#include "BOShrewdBoss.generated.h"

class UStateTree;

/**
 * Shrewd Boss (중간 보스)
 * 발판(Platform) 및 지면(Ground) 페이즈를 오가는 보스.
 * (씨앗 패턴은 기획 보류로 인해 임시 제외됨)
 */
UCLASS()
class PROJECTBLACKOUT_API ABOShrewdBoss : public ABlackoutBossCharacter
{
	GENERATED_BODY()

public:
	ABOShrewdBoss();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Boss|Shrewd")
	void EnterPlatformPhase();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Boss|Shrewd")
	void EnterGroundPhase();

protected:
	virtual void OnPhaseChanged(EBossPhase NewPhase) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTree> ST_Shrewd_Phases;

	/** 발판 위에 있는지 여부. StateTree Condition 등에서 참조. Replicated. */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Boss|Shrewd")
	bool bIsOnPlatform;
};
