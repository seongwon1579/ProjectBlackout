// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOProjectile.h"
#include "BOWraithArrowProjectile.generated.h"

/**
 * Wraith 정예 미니언 전용 화살 Projectile.
 * Pool reuse 시 Lifespan timer 재시작/정지 처리.
 */
UCLASS()
class PROJECTBLACKOUT_API ABOWraithArrowProjectile : public ABOProjectile
{
	GENERATED_BODY()

public:
	virtual void OnSpawnFromPool_Implementation() override;
	virtual void OnReturnToPool_Implementation() override;
};
