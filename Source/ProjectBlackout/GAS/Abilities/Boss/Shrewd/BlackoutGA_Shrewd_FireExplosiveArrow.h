// ─── 구현 내역 ───────────────────────
//  - 조성원: 곡사 폭발 화살 발사 어빌리티 — 타겟 지점 빠른 낙하 적용
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutGA_Shrewd_FireArrowBase.h"
#include "BlackoutGA_Shrewd_FireExplosiveArrow.generated.h"

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_FireExplosiveArrow : public UBlackoutGA_Shrewd_FireArrowBase
{
	GENERATED_BODY()

protected:
	virtual void LaunchProjectile(ABOProjectile* Arrow, const FVector& SpawnLocation,
	                              const FVector& TargetLocation) override;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Projectile|Parabolic",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ArcParam = 0.5f;

	// 직선 속도 폴백
	UPROPERTY(EditDefaultsOnly, Category = "Abilities|Projectile|Parabolic")
	float FallbackStraightSpeed = 3000.f;
	
	UPROPERTY(EditAnywhere, Category = "Abilities|Projectile|Parabolic", meta=(ClampMin="0.1"))
	float SpeedMultiplier = 1.3f;
};
