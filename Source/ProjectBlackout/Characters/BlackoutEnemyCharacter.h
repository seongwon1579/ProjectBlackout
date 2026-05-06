#pragma once

#include "CoreMinimal.h"
#include "BlackoutCharacterBase.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BlackoutEnemyCharacter.generated.h"

class UBOMinionData;
class UBlackoutBaseAttributeSet;

/**
 * AI 제어 적 캐릭터 베이스. ASC를 자기 자신이 소유.
 * 오브젝트 풀링 대상: IBlackoutPoolableInterface 구현.
 * ABlackoutBossCharacter는 AI/Boss 에픽에서 이 클래스를 상속해 확장.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutEnemyCharacter : public ABlackoutCharacterBase, public IBlackoutPoolableInterface
{
	GENERATED_BODY()

public:
	ABlackoutEnemyCharacter();

	virtual void BeginPlay() override;

	// ── IBlackoutPoolableInterface ──────────────────────────────────────────

	/** 풀에서 꺼낼 때: Hidden 해제, Collision 켜기, ASC 초기화(HP 복구). */
	virtual void OnSpawnFromPool_Implementation() override;

	/** 풀로 반환할 때: Hidden 처리, Collision 끄기, ASC 전체 클리어. */
	virtual void OnReturnToPool_Implementation() override;

protected:
	/** 모든 적/보스가 공유하는 기본 전투 어트리뷰트 세트. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Attributes")
	TObjectPtr<UBlackoutBaseAttributeSet> BaseAttributeSet;

	/** 미니언 스탯 데이터. BP 서브클래스(BP_RootHollow 등)에서 지정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Data")
	TObjectPtr<UBOMinionData> MinionData;
};
