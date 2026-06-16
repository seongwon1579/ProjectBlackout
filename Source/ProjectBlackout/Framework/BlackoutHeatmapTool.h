// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlackoutHeatmapTool.generated.h"

class UStaticMesh;
class UMaterialInterface;

// Python 이 export 한 그리드 미러 
// Values = row-major rox = x , col = y , 값 = 체류시간

USTRUCT(BlueprintType)
struct FBlackoutHeatmapGrid
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly) int32 RowsX=0;
	UPROPERTY(BlueprintReadOnly) int32 ColsY=0;
	UPROPERTY(BlueprintReadOnly) float CellSize = 0.0f;
	UPROPERTY(BlueprintReadOnly) double WorldXMin = 0.0;
	UPROPERTY(BlueprintReadOnly) double WorldXMax = 0.0;
	UPROPERTY(BlueprintReadOnly) double WorldYMin = 0.0;
	UPROPERTY(BlueprintReadOnly) double WorldYMax = 0.0;
	UPROPERTY(BlueprintReadOnly) float MaxValue =0.0f; 
	UPROPERTY(BlueprintReadOnly) FString LevelName;
	UPROPERTY(BlueprintReadOnly) TArray<float> Values;
	// 셀별 평균 플레이어 Z (zgrid.csv). 비었으면 raycast 폴백. row-major, Values 와 동일 차원
	UPROPERTY(BlueprintReadOnly) TArray<float> CellZ;

};

/**
 * 텔레메트리 히트맵 에디터 툴 
 */
UCLASS()
class PROJECTBLACKOUT_API
	UBlackoutHeatmapTool : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
#if WITH_EDITOR
public:
	UFUNCTION(BlueprintCallable , Category="Blackout|Telemetry")
	static bool LoadHeatmapGrid(const FString& GridCsvPath , FBlackoutHeatmapGrid& OutGrid);
	
	UFUNCTION(BlueprintCallable, Category="Blackout|Telemetry")
	static AActor* BuildHeatmap(const FBlackoutHeatmapGrid& Grid, UStaticMesh* CellMesh,
		UMaterialInterface* CellMaterial, float FloorZ, float HeightScale, float MinThreshold,
		bool bSnapToGround );
	
	UFUNCTION(BlueprintCallable , Category="Blackout|Telemetry"  ,meta=(WorldContext="WorldContextObject"))
	static int32 ClearHeatmaps(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category="Blackout|Telemetry")
	static FString PickGridCsvFile(const FString& DefaultPath);
	
#endif
};
