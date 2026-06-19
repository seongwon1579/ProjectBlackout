// ─── 구현 내역 ───────────────────────
//  - 최승현: 풀링 미니언 베이스 — 풀 인터페이스 / MinionData 기반 ASC 초기화 / 머리 위 체력바 / 사망 dissolve 통합
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutEnemyCharacter.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BlackoutMinionCharacter.generated.h"

class UBODissolveComponent;
class UBOMinionData;
class UBOMinionHealthBarComponent;

/**
 * 풀링되는 일반/정예 미니언 베이스. ABlackoutEnemyCharacter 의 ASC/AttributeSet 공통을 상속하고
 * 풀 인터페이스 / MinionData 기반 ASC 초기화 / 머리 위 체력바 / 사망 dissolve 까지 미니언 전용 흐름을 모은다.
 * 보스(ABlackoutBossCharacter)는 풀을 쓰지 않으므로 이 클래스를 거치지 않는다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutMinionCharacter : public ABlackoutEnemyCharacter, public IBlackoutPoolableInterface
{
	GENERATED_BODY()

public:
	ABlackoutMinionCharacter();

	virtual void BeginPlay() override;

	// ── IBlackoutPoolableInterface ──────────────────────────────────────────

	/** 풀에서 꺼낼 때: 사망/다운 플래그 리셋 + Hidden/Collision/Tick 복구 + MinionData 재초기화 + 체력바/dissolve/AI 복구. */
	virtual void OnSpawnFromPool_Implementation() override;

	/** 풀로 반환할 때: 체력바 숨김 + Hidden/Collision/Tick 끄기 + ASC 전체 클리어. */
	virtual void OnReturnToPool_Implementation() override;

protected:
	virtual void OnDeath() override;

	/** 미니언 스탯 데이터. BP 서브클래스(BP_RootHollow 등)에서 지정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Data")
	TObjectPtr<UBOMinionData> MinionData;

	/** 사망 dissolve 연출 + 서버 완료 통지. 튜닝 값은 이 컴포넌트에서 설정. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Dissolve")
	TObjectPtr<UBODissolveComponent> DissolveComponent;

	/** 머리 위 월드 위젯 체력바. BP 에서 WidgetClass / 높이 / 크기 지정. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|UI")
	TObjectPtr<UBOMinionHealthBarComponent> HealthBarComponent;

private:
	/** MinionData 기반 HP set + GA 부여. BeginPlay / OnSpawnFromPool 공통. */
	void ApplyMinionDataToASC();

	/** 서버 dissolve 완료(컴포넌트 통지) → 풀 반환. */
	void HandleDissolveFinished();
};
