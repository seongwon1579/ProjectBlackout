// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 김민영: 맵 로드 완료 시 사용자 설정(그래픽/오디오/입력) 자동 재적용 서브시스템
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BlackoutUserSettingsSubsystem.generated.h"

/**
 * 맵 로드가 완료될 때마다 사용자 설정(그래픽/오디오/입력)을 자동으로 재적용하는 서브시스템입니다.
 *
 * 그래픽 CVar는 엔진 전역이라 트래블 후에도 유지되지만, 오디오 SoundMix override는
 * OpenLevel 시 오디오 디바이스 flush로 소실되므로 월드 시작마다 재적용이 필요합니다.
 * LV_Entry를 포함한 모든 게임 월드 로드 완료 시점에 일괄 적용됩니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutUserSettingsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 전용 서버에서는 그래픽/오디오 적용이 무의미하므로 서브시스템을 생성하지 않습니다. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	/** 맵 로드 완료 콜백. 이 GameInstance에 속한 게임 월드에 한해 설정을 재적용합니다. */
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);

	FDelegateHandle PostLoadMapHandle;
};
