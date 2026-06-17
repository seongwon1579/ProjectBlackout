// ─── 구현 내역 ───────────────────────
//  - 최승현: Wraith 점멸 / 근접공격 / Phase B StateTree 통합 및 근접 감지 반경 추가
//  - 김민영: Root Wraith 엘리트 미니언 클래스와 스폰 GCN 호출 주체 캡슐화 / BP 노출
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutMinionCharacter.h"
#include "Templates/SubclassOf.h"
#include "BORootWraith.generated.h"

class UStateTree;
class ABOProjectile;

/**
 * Root Wraith 엘리트 미니언.
 * 순수 StateTree를 이용해 원거리 2연사 및 시야 밖 점멸을 수행.
 */
UCLASS()
class PROJECTBLACKOUT_API ABORootWraith : public ABlackoutMinionCharacter
{
	GENERATED_BODY()

public:
	ABORootWraith();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|AI")
	TObjectPtr<UStateTree> ST_RootWraith;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> ArrowProjectileClass;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Blackout|Combat")
	float MeleeDetectRadius= 300.0f;
	
	/** 이동속도 */
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Blackout|Movement" ,meta=(ClampMin=0.0f , UIMin=0.0f , UIMax=300.0f))
	float DefaultFlySpeed = 75.f;
	
};
