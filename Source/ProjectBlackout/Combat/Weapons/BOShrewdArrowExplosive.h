// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Shrewd 곡사형 폭발 화살 발사체(중력 적용 발사·복제) 구현
//  - 김민영: 착탄 폭발 데미지 및 폭발 Gameplay Cue 실행 경로 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOProjectile.h"
#include "BOShrewdArrowExplosive.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABOShrewdArrowExplosive : public ABOProjectile
{
	GENERATED_BODY()

public:
	ABOShrewdArrowExplosive();

protected:
	
	virtual void Launch(const FVector& Velocity) override;
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                   FVector NormalImpulse, const FHitResult& Hit) override;
	
	void ExecuteExplosionCue(const FHitResult& Hit);
	void ApplyImpactDamage(const FHitResult& Hit);
	
	UPROPERTY(EditDefaultsOnly, Category = "Blackout|Arrow", meta = (ClampMin = "0.0"))
	float ExplosionRadius = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Projectile", meta=(ClampMin="0.1"))
	float SpeedMultiplier = 1.5f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FGameplayTag ExplosionCueTag;
	
	UPROPERTY(EditAnywhere, Category = "Blackout|Debug")
	bool bShowDebugExplosion = true;
};
