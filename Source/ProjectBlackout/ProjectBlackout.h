// Copyright Epic Games, Inc. All Rights Reserved.

// ─── 구현 내역 ───────────────────────
//  - 김민영: UENUM 리플렉션 등록을 위해 기본 모듈을 커스텀 모듈 클래스(FProjectBlackoutModule)로 전환
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogProjectBlackout, Log, All);

class FProjectBlackoutModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};