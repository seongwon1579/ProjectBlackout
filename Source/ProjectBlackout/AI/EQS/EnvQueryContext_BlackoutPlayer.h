// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: EQS 쿼리에 플레이어들을 컨텍스트로 제공하는 EnvQueryContext
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_BlackoutPlayer.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API
	UEnvQueryContext_BlackoutPlayer : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
