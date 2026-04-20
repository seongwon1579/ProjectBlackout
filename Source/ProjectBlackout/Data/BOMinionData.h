#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BOMinionData.generated.h"

/**
 * 미니언(Root Hollow / Root Wraith) 스탯 및 패턴 데미지 데이터.
 * ABlackoutEnemyCharacter::BeginPlay에서 어트리뷰트 초기화에 참조.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBOMinionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MovementSpeed = 400.f;

	/**
	 * 패턴별 기본 데미지 배율 맵.
	 * Key: 패턴 어빌리티 태그 (e.g. GA.Minion.MeleeAttack)
	 * Value: 기본 데미지 값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Abilities")
	TMap<FGameplayTag, float> AbilityDamageMap;
};
