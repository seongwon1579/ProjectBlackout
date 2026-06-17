// ─── 구현 내역 ───────────────────────
//  - 김민영: 소모품 정적 정의 DataAsset 전환 + 태그 기반 효과 수치(식별 태그·UI·수량 한도·쿨다운·사용 GA/GE·효과 맵) 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "BOConsumableData.generated.h"

class UTexture2D;

/**
 * 소모품별 정적 정의 데이터.
 * 현재 소지 수량은 PlayerState가 복제하고, 실제 사용 로직은 UseAbility / GameplayEffect가 담당합니다.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBOConsumableData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 소모품을 식별하는 GameplayTag. 예: Item.Consumable.BloodRoot */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable")
	FGameplayTag ConsumableTag;

	/** HUD와 툴팁에 표시할 이름입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|UI")
	FText DisplayName;

	/** HUD와 툴팁에 표시할 설명입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|UI", meta = (MultiLine = true))
	FText Description;

	/** HUD 소모품 슬롯에 표시할 아이콘입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|UI")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 전투 진입 또는 체크포인트 보정 시 최소 지급할 기본 수량입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|Count", meta = (ClampMin = 0))
	int32 InitialCount = 1;

	/** 획득, 보상, 재보급으로 보유할 수 있는 최대 수량입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|Count", meta = (ClampMin = 0))
	int32 MaxCount = 99;

	/** 사용 후 같은 소모품을 다시 사용할 수 있기까지의 시간입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|Effect", meta = (ClampMin = 0.f))
	float Cooldown = 0.f;

	/** 사용 시 발동할 어빌리티입니다. 소모품별 입력/애니메이션/차감/효과 적용 흐름을 담당합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|Effect")
	TSubclassOf<UGameplayAbility> UseAbility;

	/** UseAbility가 적용할 기본 GameplayEffect입니다. 효과가 완전히 커스텀이라면 비워둘 수 있습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|Effect")
	TSubclassOf<UGameplayEffect> GameplayEffect;

	/**
	 * 소모품별 추가 효과 수치입니다.
	 * 예: Data.Consumable.HealAmount, Data.Consumable.Duration, Data.Consumable.StaminaCostMultiplier 등.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumable|Effect")
	TMap<FGameplayTag, float> EffectMagnitudes;
};
