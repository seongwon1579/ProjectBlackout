// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Weapons/BOWraithArrowProjectile.h"

void ABOWraithArrowProjectile::OnSpawnFromPool_Implementation()
{
	Super::OnSpawnFromPool_Implementation();

	// Pool reuse 시 BP 의 Initial Life Span 으로 timer 재시작
	SetLifeSpan(InitialLifeSpan);
}

void ABOWraithArrowProjectile::OnReturnToPool_Implementation()
{
	Super::OnReturnToPool_Implementation();

	// Pool 대기 동안 timer 정지 - destroy 막음
	SetLifeSpan(0.0f);
}
