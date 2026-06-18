// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: EQS 질의로 텔레포트 목적지 결정
//  - 최승현: 상위 후보 중 랜덤 선택(RandomBest25Pct)으로 같은 위치 반복 방지
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_TeleportBase.h"
#include "BlackoutGA_Shrewd_TeleportByEQS.generated.h"

/**
 * 
 */
class UEnvQuery;
struct FEnvQueryResult;

UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Shrewd_TeleportByEQS : public UBlackoutGA_Shrewd_TeleportBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="Blackout")
	TObjectPtr<UEnvQuery> TeleportQuery;

	virtual void StartResolveDestination() override;

	
private:
	void OnEQSFinished(TSharedPtr<FEnvQueryResult> Result);
};
