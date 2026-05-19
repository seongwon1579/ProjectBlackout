#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BlackoutArenaResettable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UBlackoutArenaResettableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 보스 아레나(구역)가 자기 상태를 결정적으로 초기화하는 단위로서 구현하는 인터페이스.
 * ABlackoutBattleGameMode 가 파티 전멸 / 체크포인트 복귀 시 CurrentArena->ResetArena() 호출.
 * 파괴물(기둥) 복원도 구현자가 ResetArena 내에서 함께 처리한다.
 * 흐름 권위(GameMode)와 아레나 상태를 분리하기 위한 인터페이스다.
 */
class PROJECTBLACKOUT_API IBlackoutArenaResettableInterface
{
	GENERATED_BODY()

public:
	/** 아레나를 결정적 초기 상태로 복원 (보스 / 미니언 풀 / 타이머 / 인플라이트 / GAS / 파괴물). */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Arena")
	void ResetArena();
	virtual void ResetArena_Implementation() {}

	/** 리셋이 결정적인지 자기보고. 파괴물 복원이 결정적이지 않으면 false 를 반환한다. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Arena")
	bool IsResetDeterministic() const;
	virtual bool IsResetDeterministic_Implementation() const { return true; }
};
