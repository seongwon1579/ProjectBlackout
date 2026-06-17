// ─── 구현 내역 ───────────────────────
//  - 김민영: 풀 스폰/반환 시점 콜백(OnSpawnFromPool·OnReturnToPool)을 받는 풀링 대상 공용 인터페이스 정의
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BlackoutPoolable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UBlackoutPoolableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 오브젝트 풀링 대상 액터가 구현해야 하는 인터페이스.
 * UBlackoutPoolSubsystem이 GetFromPool/ReturnToPool 시 호출.
 */
class PROJECTBLACKOUT_API IBlackoutPoolableInterface
{
	GENERATED_BODY()

public:
	/** 풀에서 꺼낼 때 호출. Hidden 해제, Collision 활성화, 스탯 리셋 수행. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Pool")
	void OnSpawnFromPool();
	virtual void OnSpawnFromPool_Implementation() {}

	/** 풀로 반환할 때 호출. Destroy 대신 사용. Hidden, Collision Off, Tick Off 처리. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Pool")
	void OnReturnToPool();
	virtual void OnReturnToPool_Implementation() {}
};
