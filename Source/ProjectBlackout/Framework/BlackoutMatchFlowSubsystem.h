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
	
	// 정원 , GameMode 가 InitGame 에서 읽어서 MaxPlayers 설정
	int32 GetExpectedPlayers() const {return ExpectedPlayers; }
	
	// 싱글/멀티 진입시 정원 , 1 미만은 1로 클램프
	void SetExpectedPlayers(int32 InCount) {ExpectedPlayers = FMath::Max(1 , InCount); }
	
	// 정원별 보스 체력 배율 (1인 기준) 
	float GetBossHealthMultiplier() const;
	
private:
	int32 CurrentStageIndex=0;
	
	// 멀티 기본 4 , 싱글 1
	int32 ExpectedPlayers=4;
	
	// 1인 1.0 , 4인 3.5  ( 2 / 3 인은 프로젝트 X )
	TMap<int32, float> BossHealthMultiplierByPlayerCount = {{1,1.0f} , {4,3.5f}};
};
