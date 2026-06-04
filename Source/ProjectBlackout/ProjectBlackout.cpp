// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectBlackout.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FProjectBlackoutModule, ProjectBlackout, "ProjectBlackout");

DEFINE_LOG_CATEGORY(LogProjectBlackout);

void FProjectBlackoutModule::StartupModule()
{
	// AbilitySystemGlobals::InitGlobalData()를 여기서 호출하면 안 됩니다.
	// StartupModule은 엔진 초기 로드(CloseDisregardForGC 이전)에 실행되므로, 이 시점에 호출하면
	// AbilitySystemGlobals/GameplayCueManager가 DisregardForGC 풀에 들어갑니다. 이후 런타임에
	// 스폰된 GameplayCueNotify_Actor를 CueManager가 참조하면 GC 가정 위반으로 치명적 크래시가 발생합니다
	// (VerifyGCAssumptions). UE5.4+에서는 bInitGlobalDataOnPostEngineInit(기본 true)에 의해 엔진이
	// PostEngineInit 시점에 자동으로 InitGlobalData()를 호출하므로 수동 호출이 불필요합니다.
}

void FProjectBlackoutModule::ShutdownModule()
{
}
