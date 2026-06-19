// ─── 구현 내역 ───────────────────────
//  - 김민영: 미니언 기본 스탯(체력·이동속도)·패턴별 데미지 배율 맵 데이터 정의
//  - 최승현: ASC에 일괄 부여할 어빌리티 목록(GrantedAbilities) 추가
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BOMinionData.generated.h"

class UGameplayAbility;
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
	
	/**
	 * ASC 에 부여할 어빌리티 클래스 목록.
	 * BlackoutEnemyCharacter::BeginPlay 에서 GiveDefaultAbilities 로 일괄 부여.
	 * 미니언이 GA 사용 시에만 채움 (Hollow 빈 배열 / Wraith 사용).
	 */
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Blackout|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
};
