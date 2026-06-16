// Fill out your copyright notice in the Description page of Project Settings.

#include "BlackoutHeatmapTool.h"

#if WITH_EDITOR

#include "Components/InstancedStaticMeshComponent.h"
#include "Core/BlackoutLog.h"
#include "Core/BlackoutLogCategories.h"
#include "Misc/FileHelper.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization//JsonSerializer.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/Paths.h"
#include "CollisionQueryParams.h"

	static const FName GHeatmapActorTag(TEXT("BlackoutHeatmap"));

static constexpr float PLAYER_CAPSULE_HALF = 88.f; // 샘플 Z(캡슐 중심) → 발밑 보정

static bool ParseHeatmapGrid(const FString &CsvBody, const FString &MetaBody, const FString &ZBody, FBlackoutHeatmapGrid &OutGrid)
{
	OutGrid = FBlackoutHeatmapGrid();

	TSharedPtr<FJsonObject> Meta;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(MetaBody);
	if (!FJsonSerializer::Deserialize(Reader, Meta) || !Meta.IsValid())
	{
		BO_LOG_CORE(Error, "Heatmap Parse — meta.json 파싱 실패");
		return false;
	}
	Meta->TryGetStringField(TEXT("level_name"), OutGrid.LevelName);
	OutGrid.CellSize = Meta->GetNumberField(TEXT("cell_size"));
	const TSharedPtr<FJsonObject> *World = nullptr;
	if (!Meta->TryGetObjectField(TEXT("world"), World) || !World)
	{
		BO_LOG_CORE(Error, "Heatmap Parse — meta.world 없음");
		return false;
	}
	OutGrid.WorldXMin = (*World)->GetNumberField(TEXT("x_min"));
	OutGrid.WorldXMax = (*World)->GetNumberField(TEXT("x_max"));
	OutGrid.WorldYMin = (*World)->GetNumberField(TEXT("y_min"));
	OutGrid.WorldYMax = (*World)->GetNumberField(TEXT("y_max"));

	TArray<FString> Lines;
	CsvBody.ParseIntoArrayLines(Lines, true);
	if (Lines.Num() == 0)
	{
		BO_LOG_CORE(Error, "Heatmap Parse — grid.csv 비어있음");
		return false;
	}

	OutGrid.RowsX = Lines.Num();
	TArray<FString> FirstCols;
	Lines[0].ParseIntoArray(FirstCols, TEXT(","), false);
	OutGrid.ColsY = FirstCols.Num();
	OutGrid.Values.Reserve(OutGrid.RowsX * OutGrid.ColsY);

	for (const FString &Line : Lines)
	{
		TArray<FString> Cols;
		Line.ParseIntoArray(Cols, TEXT(","), false);
		for (int32 j = 0; j < OutGrid.ColsY; ++j)
		{
			const float V = Cols.IsValidIndex(j) ? FCString::Atof(*Cols[j]) : 0.0f;
			OutGrid.Values.Add(V);
			OutGrid.MaxValue = FMath::Max(OutGrid.MaxValue, V);
		}
	}

	// zgrid (셀별 평균 Z) — 있으면 파싱. 렌더 셀(count>0)은 항상 유효. 없으면 비워 raycast 폴백.
	if (!ZBody.IsEmpty())
	{
		TArray<FString> ZLines;
		ZBody.ParseIntoArrayLines(ZLines, true);
		OutGrid.CellZ.Reserve(OutGrid.RowsX * OutGrid.ColsY);
		for (const FString &Line : ZLines)
		{
			TArray<FString> Cols;
			Line.ParseIntoArray(Cols, TEXT(","), false);
			for (int32 j = 0; j < OutGrid.ColsY; ++j)
			{
				OutGrid.CellZ.Add(Cols.IsValidIndex(j) ? FCString::Atof(*Cols[j]) : 0.0f);
			}
		}
		if (OutGrid.CellZ.Num() != OutGrid.Values.Num())
		{
			BO_LOG_CORE(Warning, "Heatmap Parse — zgrid 크기 불일치, raycast 폴백");
			OutGrid.CellZ.Reset();
		}
	}

	double MetaRows = 0.0, MetaCols = 0.0;
	Meta->TryGetNumberField(TEXT("rows_x"), MetaRows);
	Meta->TryGetNumberField(TEXT("cols_y"), MetaCols);
	if (((int32)MetaRows && (int32)MetaRows != OutGrid.RowsX) || ((int32)MetaCols && (int32)MetaCols != OutGrid.ColsY))
	{
		BO_LOG_CORE(Warning, "Heatmap Parse — meta(%dx%d) ≠ csv(%dx%d)",
					(int32)MetaRows, (int32)MetaCols, OutGrid.RowsX, OutGrid.ColsY);
	}

	return true;
}

bool UBlackoutHeatmapTool::LoadHeatmapGrid(const FString &GridCsvPath,
										   FBlackoutHeatmapGrid &OutGrid)
{
	FString CsvBody;
	if (!FFileHelper::LoadFileToString(CsvBody, *GridCsvPath))
	{
		BO_LOG_CORE(Error, "Heatmap Load — grid.csv 못 읽음: %s", *GridCsvPath);
		return false;
	}
	FString MetaPath = GridCsvPath;
	MetaPath.RemoveFromEnd(TEXT(".csv"));
	MetaPath += TEXT(".meta.json");
	FString MetaBody;
	if (!FFileHelper::LoadFileToString(MetaBody, *MetaPath))
	{
		BO_LOG_CORE(Error, "Heatmap Load — meta.json 못 읽음: %s", *MetaPath);
		return false;
	}

	// zgrid (....dwell.zgrid.csv) — 없으면 빈 문자열 → raycast 폴백
	FString ZPath = GridCsvPath;
	ZPath.RemoveFromEnd(TEXT(".grid.csv"));
	ZPath += TEXT(".zgrid.csv");
	FString ZBody;
	FFileHelper::LoadFileToString(ZBody, *ZPath);

	const bool bOk = ParseHeatmapGrid(CsvBody, MetaBody, ZBody, OutGrid);
	if (bOk)
	{
		BO_LOG_CORE(Log, "Heatmap Load — %s %dx%d max=%.1f cells=%d",
					*OutGrid.LevelName, OutGrid.RowsX, OutGrid.ColsY,
					OutGrid.MaxValue, OutGrid.Values.Num());
	}
	return bOk;
}

AActor *UBlackoutHeatmapTool::BuildHeatmap(const FBlackoutHeatmapGrid &Grid,
										   UStaticMesh *CellMesh, UMaterialInterface *CellMaterial, float FloorZ,
										   float HeightScale, float MinThreshold, bool bSnapToGround)
{
	UWorld *World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World || !CellMesh || Grid.Values.Num() == 0 || Grid.MaxValue <= 0.f)
	{
		BO_LOG_CORE(Error, "Heatmap Build — World/메시/그리드 무효");
		return nullptr;
	}

	const double CellX = (Grid.WorldXMax - Grid.WorldXMin) / FMath::Max(Grid.RowsX, 1);
	const double CellY = (Grid.WorldYMax - Grid.WorldYMin) / FMath::Max(Grid.ColsY, 1);

	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |= RF_Transient;
	AActor *HeatmapActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);
	if (!HeatmapActor)
	{
		return nullptr;
	}
	HeatmapActor->Tags.Add(GHeatmapActorTag);

	UInstancedStaticMeshComponent *HISM = NewObject<UInstancedStaticMeshComponent>(HeatmapActor);
	HeatmapActor->SetRootComponent(HISM);
	HISM->SetStaticMesh(CellMesh);
	HISM->NumCustomDataFloats = 1;
	HISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HISM->RegisterComponent();
	if (CellMaterial)
	{
		// 등록 후 적용 — NewObject ISM 은 RegisterComponent 전 SetMaterial 이 안 붙음
		HISM->SetMaterial(0, CellMaterial);
	}

	// FloorZ(아레나 바닥 근사) 살짝 위에서 시작 — 위쪽 절벽/오버행에 막대가 붙는 것 방지.
	// 아래로는 넉넉히 — 협곡/계단 등 낮은 바닥까지 포착.
	const float TraceTopZ = FloorZ + 2000.f;
	const float TraceBotZ = FloorZ - 50000.f;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(HeatmapGroundSnap), true);
	TraceParams.AddIgnoredActor(HeatmapActor);

	int32 Count = 0;
	for (int32 i = 0; i < Grid.RowsX; ++i)
	{
		for (int32 j = 0; j < Grid.ColsY; ++j)
		{
			const float V = Grid.Values[i * Grid.ColsY + j];
			if (V <= MinThreshold)
			{
				continue;
			}
			const float Norm = V / Grid.MaxValue;
			const double WorldX = Grid.WorldXMin + (i + 0.5) * CellX;
			const double WorldY = Grid.WorldYMin + (j + 0.5) * CellY;
			const float Height = FMath::Max(Norm * HeightScale, 1.0f);

			const int32 CellIdx = i * Grid.ColsY + j;
			float BaseZ = FloorZ;
			if (Grid.CellZ.IsValidIndex(CellIdx) && !FMath::IsNaN(Grid.CellZ[CellIdx]))
			{
				// 데이터 기반: 캡처된 플레이어 Z(캡슐 중심) → 발밑. raycast 불필요(볼륨/메시 간섭 0)
				BaseZ = Grid.CellZ[CellIdx] - PLAYER_CAPSULE_HALF;
			}
			else if (bSnapToGround)
			{
				FHitResult Hit;
				const FVector Start(WorldX, WorldY, TraceTopZ);
				const FVector End(WorldX, WorldY, TraceBotZ);
				if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams))
				{
					BaseZ = Hit.ImpactPoint.Z;
				}
			}

			FTransform T;
			T.SetLocation(FVector(WorldX, WorldY, BaseZ + Height * 0.5));
			T.SetScale3D(FVector(CellX / 100.0, CellY / 100.0, Height / 100.0));
			const int32 Idx = HISM->AddInstance(T, true);
			HISM->SetCustomDataValue(Idx, 0, Norm, true);
			++Count;
		}
	}

	HISM->MarkRenderStateDirty();
	BO_LOG_CORE(Log, "Heatmap Build — %s 막대 %d개 (snap=%d, max dwell=%.1f)",
				*Grid.LevelName, Count, bSnapToGround ? 1 : 0, Grid.MaxValue);
	return HeatmapActor;
}

int32 UBlackoutHeatmapTool::ClearHeatmaps(UObject *WorldContextObject)
{
	UWorld *World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	if (!World)
	{
		return 0;
	}

	TArray<AActor *> ToRemove;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->Tags.Contains(GHeatmapActorTag))
		{
			ToRemove.Add(*It);
		}
	}
	for (AActor *A : ToRemove)
	{
		A->Destroy();
	}

	BO_LOG_CORE(Log, "Heatmap Clear — %d개 제거", ToRemove.Num());
	return ToRemove.Num();
}

FString UBlackoutHeatmapTool::PickGridCsvFile(const FString &DefaultPath)
{
	IDesktopPlatform *DP = FDesktopPlatformModule::Get();
	if (!DP)
	{
		return FString();
	}

	const void *ParentHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	TArray<FString> OutFiles;
	const bool bPicked = DP->OpenFileDialog(
		ParentHandle,
		TEXT("히트맵 grid.csv 선택"),
		DefaultPath,
		TEXT(""),
		TEXT("Heatmap Grid (*.grid.csv)|*.grid.csv|All files (*.*)|*.*"),
		EFileDialogFlags::None,
		OutFiles);

	if (!bPicked || OutFiles.Num() == 0)
	{
		return FString();
	}

	return FPaths::ConvertRelativePathToFull(OutFiles[0]);
}

#endif
