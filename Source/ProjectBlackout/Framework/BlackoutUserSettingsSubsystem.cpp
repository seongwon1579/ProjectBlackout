// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/BlackoutUserSettingsSubsystem.h"

#include "Framework/BlackoutUserSettings.h"
#include "Core/BlackoutLog.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

bool UBlackoutUserSettingsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	return !IsRunningDedicatedServer();
}

void UBlackoutUserSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UBlackoutUserSettingsSubsystem::HandlePostLoadMapWithWorld);
}

void UBlackoutUserSettingsSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	Super::Deinitialize();
}

void UBlackoutUserSettingsSubsystem::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
	// 다른 GameInstance(예: 동일 프로세스의 별도 PIE 인스턴스)의 월드 로드는 무시합니다.
	if (!LoadedWorld || LoadedWorld->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	UBlackoutUserSettings* UserSettings = Cast<UBlackoutUserSettings>(UGameUserSettings::GetGameUserSettings());
	if (!UserSettings)
	{
		BO_LOG_CORE(Warning, "사용자 설정 자동 적용을 건너뜁니다. UBlackoutUserSettings를 찾지 못했습니다.");
		return;
	}

	// 시작 시 적용은 디스크에 다시 저장할 필요가 없으므로 bSaveSettings=false로 적용합니다.
	UserSettings->ApplyBlackoutUserSettings(false);
}
