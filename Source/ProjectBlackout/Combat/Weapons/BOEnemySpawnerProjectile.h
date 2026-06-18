// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 적 스포너 발사체(착지 후 부화 타이머로 미니언 생성, 오버랩/피직스 충돌 분기) 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOEnemyProjectile.h"
#include "BOEnemySpawnerProjectile.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API ABOEnemySpawnerProjectile : public ABOEnemyProjectile
{
	GENERATED_BODY()

public:
	ABOEnemySpawnerProjectile();
	void SetSpawnerData(const FMinionSpawnData& InData);

protected:
	virtual void SetCollisionEvent() override;

private:
	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	void Hatch();

	UPROPERTY(Transient)
	FMinionSpawnData MinionData;

	UPROPERTY(Transient)
	FTimerHandle HatchTimer;

	UPROPERTY(Transient)
	bool bHasLanded = false;
};
