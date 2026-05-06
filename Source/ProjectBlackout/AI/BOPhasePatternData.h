#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "BOPhasePatternData.generated.h"

/** 페이즈별 단일 공격 패턴 항목. */
USTRUCT(BlueprintType)
struct FBOPatternEntry
{
	GENERATED_BODY()

	/** 실행할 어빌리티 태그 (GA.Ravager.LungeAttack 등). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pattern")
	FGameplayTag AbilityTag;

	/** 사용 가능한 최소 거리 (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pattern", meta = (ClampMin = "0.0"))
	float MinDistance = 0.f;

	/** 사용 가능한 최대 거리 (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pattern", meta = (ClampMin = "0.0"))
	float MaxDistance = 1000.f;

	/** 가중치. 클수록 선택 확률이 높아진다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pattern", meta = (ClampMin = "0.0"))
	float Weight = 1.f;

	/** 이 패턴이 선택된 후 재사용 불가 시간 (초). 0이면 쿨다운 없음. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pattern", meta = (ClampMin = "0.0"))
	float Cooldown = 3.f;
};

/**
 * BT 패턴 선택 Task(BTTask_SelectPattern)에서 참조하는 페이즈별 패턴 목록.
 * 에디터에서 거리·가중치·쿨다운을 직접 조정한다.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBOPhasePatternData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Pattern")
	TArray<FBOPatternEntry> Patterns;
};
