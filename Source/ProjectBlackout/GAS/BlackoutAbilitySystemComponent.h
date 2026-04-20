#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
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
	 * 서버 전용. 모든 GA와 GE를 제거. 미니언 풀 반환(OnReturnToPool) 시 ASC 초기화에 사용.
	 */
	void ClearAllAbilitiesAndEffects();
};
