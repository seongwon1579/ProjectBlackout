// Copyright Epic Games, Inc. All Rights Reserved.

// ─── 구현 내역 ───────────────────────
//  - (없음) Epic Third Person 템플릿 기본 GameMode — 프로젝트 기능 미사용, 정리 대상 스캐폴딩
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectBlackoutGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AProjectBlackoutGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AProjectBlackoutGameMode();
};



