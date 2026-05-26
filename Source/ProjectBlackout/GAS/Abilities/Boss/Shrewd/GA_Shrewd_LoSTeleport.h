#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"
#include "GA_Shrewd_LoSTeleport.generated.h"

/**
 * LoS 차단 시 즉시 점멸 후 근접 콤보 자동 연계.
 * 구현 흐름:
 *   1. Teleport: NavMesh에서 타겟 인접 유효 지점으로 SetActorLocation 이동
 *   2. 딜레이(PostTeleportComboDelay) 대기 (착지 예열)
 *   3. FollowUpAbilityTag 에 해당하는 GA를 ASC에서 TryActivateAbilitiesByTag로 연계 발동
 *      (기본값: GA.Shrewd.MeleeCombo)
 */
UCLASS()
class PROJECTBLACKOUT_API UGA_Shrewd_LoSTeleport : public UBlackoutGA_Ravager_Base
{
	GENERATED_BODY()

public:
	UGA_Shrewd_LoSTeleport();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * 텔레포트 완료 후 자동으로 연계 발동할 GA 태그.
	 * 기본값은 GA.Shrewd.MeleeCombo. 에디터에서 변경 가능.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	FGameplayTag FollowUpAbilityTag;

	/**
	 * 텔레포트 착지 후 연계 GA 발동까지의 대기 시간 (초).
	 * 착지 모션 예열 시간 확보용.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability", meta = (ClampMin = 0.f))
	float PostTeleportComboDelay = 0.15f;
};
