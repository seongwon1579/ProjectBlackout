// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/BlackoutEnemyGameplayAbility.h"

UBlackoutEnemyGameplayAbility::UBlackoutEnemyGameplayAbility()
{
	// 보스 AI는 서버 Authority 전용 — 클라이언트 예측 불필요
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;

	// GA 인스턴스별 상태(타이머, 단계 카운터 등)가 필요하므로 인스턴스 유지
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
