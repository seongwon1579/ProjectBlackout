#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutAbilitySystemComponent.generated.h"

/**
 * 프로젝트 전용 ASC.
 * 플레이어는 ABlackoutPlayerState, 적/보스는 ACharacter 자신이 소유.
 */
UCLASS(ClassGroup = "Blackout", meta = (BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/**
	 * 서버 전용. CharacterData의 GrantedAbilities 배열을 순회해 ASC에 일괄 부여.
	 * ABlackoutPlayerCharacter::PossessedBy, ABlackoutEnemyCharacter::BeginPlay 에서 호출.
	 */
	void GiveDefaultAbilities(const TArray<TSubclassOf<UGameplayAbility>>& Abilities);

	/**
	 * 로컬 입력 ID를 기준으로 대응되는 GA를 활성화하거나 활성 어빌리티에 입력 pressed 이벤트를 전달합니다.
	 */
	void HandleAbilityInputPressed(EBlackoutAbilityInputID InputID);

	/**
	 * 로컬 입력 ID를 기준으로 활성 중인 GA에 입력 released 이벤트를 전달합니다.
	 */
	void HandleAbilityInputReleased(EBlackoutAbilityInputID InputID);

	/**
	 * 서버 전용. 모든 GA와 GE를 제거. 미니언 풀 반환(OnReturnToPool) 시 ASC 초기화에 사용.
	 */
	void ClearAllAbilitiesAndEffects();
};
