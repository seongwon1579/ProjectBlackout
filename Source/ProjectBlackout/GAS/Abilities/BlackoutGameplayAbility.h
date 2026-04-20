#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "BlackoutTypes.h"
#include "BlackoutGameplayAbility.generated.h"

/**
 * 프로젝트 전용 GA 베이스 클래스.
 * 플레이어 GA(GA_Dodge, GA_FireWeapon 등)와 보스 패턴 GA 모두 이 클래스에서 파생.
 */
UCLASS(Abstract)
class PROJECTBLACKOUT_API UBlackoutGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGameplayAbility();

	/** EnhancedInput 바인딩용 입력 ID. InputComponent에서 AbilitySpec과 매핑. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Ability")
	EBlackoutAbilityInputID InputID = EBlackoutAbilityInputID::None;
};
