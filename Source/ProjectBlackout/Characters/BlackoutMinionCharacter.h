#pragma once

#include "CoreMinimal.h"
#include "BlackoutEnemyCharacter.h"
#include "BlackoutMinionCharacter.generated.h"

class UBODissolveComponent;

/**
 * 풀링되는 일반/정예 미니언 베이스. ABlackoutEnemyCharacter 의 ASC/풀 인터페이스를 상속하고
 * 사망 시 행동 정지 + dissolve 연출(UBODissolveComponent) + 풀 반환을 조율한다.
 * 보스(ABlackoutBossCharacter)는 dissolve 가 없으므로 ABlackoutEnemyCharacter 를 직접 상속.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutMinionCharacter : public ABlackoutEnemyCharacter
{
	GENERATED_BODY()

public:
	ABlackoutMinionCharacter();

	virtual void BeginPlay() override;

	/** 풀에서 꺼낼 때: Super(HP/Collision) + 사망상태/이동/AI 복구 + dissolve 리셋. */
	virtual void OnSpawnFromPool_Implementation() override;

protected:
	virtual void OnDeath() override;

	/** 사망 dissolve 연출 + 서버 완료 통지. 튜닝 값은 이 컴포넌트에서 설정. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Dissolve")
	TObjectPtr<UBODissolveComponent> DissolveComponent;

private:
	/** 서버 dissolve 완료(컴포넌트 통지) → 풀 반환. */
	void HandleDissolveFinished();
};
