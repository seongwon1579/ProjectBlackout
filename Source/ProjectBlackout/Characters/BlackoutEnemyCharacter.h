// ─── 구현 내역 ───────────────────────
//  - 김민영: AI 적 캐릭터 베이스 — 자기 자신이 ASC 를 소유하는 적 공통 클래스
//  - 최승현: 적/보스 공통 기본 전투 어트리뷰트 세트 부착 + 진영 구분 외곽선(스텐실 PP)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutCharacterBase.h"
#include "BlackoutEnemyCharacter.generated.h"

class UBlackoutBaseAttributeSet;

/**
 * AI 제어 적 캐릭터 베이스. ASC 를 자기 자신이 소유.
 * 풀링 / 체력바 / 사망 dissolve / MinionData 같은 *미니언 전용 흐름* 은
 * ABlackoutMinionCharacter 가 단독으로 책임진다. 보스(ABlackoutBossCharacter)는
 * 풀을 쓰지 않으므로 이 클래스를 직접 상속하고 풀 인터페이스도 갖지 않는다.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutEnemyCharacter : public ABlackoutCharacterBase
{
	GENERATED_BODY()

public:
	ABlackoutEnemyCharacter();

	virtual void BeginPlay() override;

protected:
	/** 모든 적/보스가 공유하는 기본 전투 어트리뷰트 세트. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Attributes")
	TObjectPtr<UBlackoutBaseAttributeSet> BaseAttributeSet;
};
