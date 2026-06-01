// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectBlackout.h"
#include "AbilitySystemGlobals.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FProjectBlackoutModule, ProjectBlackout, "ProjectBlackout");

DEFINE_LOG_CATEGORY(LogProjectBlackout);

void FProjectBlackoutModule::StartupModule()
{
	// GameplayCue 매니저가 설정된 Cue 경로를 서버/클라이언트 양쪽에서 사용할 수 있게 초기화합니다.
	UAbilitySystemGlobals::Get().InitGlobalData();
}

void FProjectBlackoutModule::ShutdownModule()
{
}
