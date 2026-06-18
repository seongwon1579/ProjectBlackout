// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager가 대상을 끌어당기는 Pull 패턴 데이터를 적용받는 대상용 인터페이스(ApplyPull) 정의
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Data/BORavagerPatternData.h"

#include "BlackoutPullable.generated.h"


// This class does not need to be modified.
UINTERFACE(MinimalAPI, NotBlueprintable)
class UBlackoutPullable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTBLACKOUT_API IBlackoutPullable
{
	GENERATED_BODY()

public:
	virtual void ApplyPull(const FPullData& PullData) = 0;
}; 
