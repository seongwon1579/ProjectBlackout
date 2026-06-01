// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/BlackoutTypes.h"
#include "BlackoutMatchFlowSubsystem.generated.h"

/**
 * 매치 진행상태(현재 도전 보스 스테이지)를 GameInstance 보관
 * Seamless Travel 사이 persistent -로비 StartBattle 보스맵 분기에 사용
 */
UCLASS()
class PROJECTBLACKOUT_API
	UBlackoutMatchFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	int32 GetCurrentStageIndex() const {return CurrentStageIndex; }
	
	// 보스 클리어 시 다음 스테이지로
	void AdvanceStage(){++CurrentStageIndex;}

	// 매치 종료(메인보스 클리어) 후 다음 매치를 위해 진행 인덱스 초기화
	void ResetStages(){CurrentStageIndex = 0;}

	EBossType GetCurrentBossType() const;
	
private:
	int32 CurrentStageIndex=0;
};
