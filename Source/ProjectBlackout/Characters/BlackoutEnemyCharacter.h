#pragma once

#include "CoreMinimal.h"
#include "BlackoutCharacterBase.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BlackoutEnemyCharacter.generated.h"

class UBOMinionData;
class UBlackoutBaseAttributeSet;
class UBlackoutMinionHealthBarWidget;
class UWidgetComponent;

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
	/** 월드 좌표에서 미니언 머리 위를 따라가는 체력바 컴포넌트입니다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|UI")
	TObjectPtr<UWidgetComponent> MinionHealthBarComponent;

	/** 미니언 크기에 따라 블루프린트에서 머리 위 표시 높이를 조정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|UI", meta = (ClampMin = "0.0"))
	float MinionHealthBarHeight = 120.0f;

	/** 체력바 월드 위젯의 렌더 크기입니다. BP 클래스 기본값에서 미니언별로 조정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|UI", meta = (ClampMin = "1.0"))
	FVector2D MinionHealthBarDrawSize = FVector2D(120.0f, 12.0f);

	/** 체력바 컴포넌트가 생성할 위젯 클래스입니다. BP_RootHollow 등에서 WBP_MinionHealthBar를 지정합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|UI")
	TSubclassOf<UBlackoutMinionHealthBarWidget> MinionHealthBarWidgetClass;

	/** 모든 적/보스가 공유하는 기본 전투 어트리뷰트 세트. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Attributes")
	TObjectPtr<UBlackoutBaseAttributeSet> BaseAttributeSet;

	/** 미니언 스탯 데이터. BP 서브클래스(BP_RootHollow 등)에서 지정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Data")
	TObjectPtr<UBOMinionData> MinionData;

	/** 미니언 전용 체력바 위젯을 현재 ASC 상태에 맞게 초기화합니다. */
	void InitializeMinionHealthBar();

	/** 사망/풀 반환 시 체력바가 남지 않도록 강제로 숨깁니다. */
	void HideMinionHealthBar();

	/** BP에서 지정한 체력바 위치와 크기 설정을 컴포넌트에 적용합니다. */
	void ApplyMinionHealthBarLayout();

	/** 기본은 MinionData가 있는 실제 미니언만 체력바를 사용합니다. 테스트 더미는 오버라이드로 허용합니다. */
	virtual bool ShouldUseMinionHealthBar() const;

	virtual void OnDeath() override;
};
