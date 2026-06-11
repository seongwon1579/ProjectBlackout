// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutClassSelectWidgetController.h"

#include "BlackoutLog.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOShotgunFirearm.h"
#include "Data/BOCharacterData.h"
#include "Data/BOCharacterRoster.h"
#include "Engine/DataTable.h"
#include "Framework/BlackoutPlayerController.h"
#include "Framework/BlackoutCharacterPreviewManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "TimerManager.h"
#include "Engine/World.h"

bool UBlackoutClassSelectWidgetController::Initialize(
	APlayerController* InPlayerController,const UBOCharacterRoster* InRoster)
{
	if (!InPlayerController || !InRoster || InRoster->Characters.Num() == 0)
	{
		BO_LOG_CORE(Warning, "ClassSelect Initialize: invalid PC or Roster");
		return false;
	}
	
	PlayerController = InPlayerController;
	Roster = InRoster;
	CurrentIndex = 0;
	return true;
}

void UBlackoutClassSelectWidgetController::BroadcastCurrentSelection()
{
	if (!Roster.IsValid() || Roster->Characters.Num() == 0)
	{
		return;
	}
	OnSelectionChanged.Broadcast(BuildDisplayData(CurrentIndex));
	UpdatePreviewPawn();
}

void UBlackoutClassSelectWidgetController::NavigateNext()
{
	if (!Roster.IsValid() || Roster->Characters.Num() == 0)
	{
		return;
	}
	CurrentIndex = (CurrentIndex+1) % Roster->Characters.Num();
	OnSelectionChanged.Broadcast(BuildDisplayData(CurrentIndex));
	UpdatePreviewPawn();
}

void UBlackoutClassSelectWidgetController::NavigatePrevious()
{
	if (!Roster.IsValid() || Roster->Characters.Num() == 0)
	{
		return;
	}
	const int32 Count = Roster->Characters.Num();
	CurrentIndex = (CurrentIndex-1 +Count) % Count;
	OnSelectionChanged.Broadcast(BuildDisplayData(CurrentIndex));
	UpdatePreviewPawn();
}

void UBlackoutClassSelectWidgetController::ConfirmSelection()
{
	if (!PlayerController.IsValid() || !Roster.IsValid() || !Roster->Characters.IsValidIndex(CurrentIndex))
	{
		return;
	}
	const UBOCharacterData* Selected = Roster->Characters[CurrentIndex];
	if (!Selected || !Selected->ClassTag.IsValid())
	{
		return;
	}
	if (ABlackoutPlayerController* BPC = Cast<ABlackoutPlayerController>(PlayerController.Get()))
	{
		BPC->Server_SelectClass(Selected->ClassTag);
	}
	OnSelectionConfirmed.Broadcast();
}

void UBlackoutClassSelectWidgetController::BeginDestroy()
{
	if (PreviewManager.IsValid())
	{
		PreviewManager->ClearPreview();
	}
	
	Super::BeginDestroy();
}

FBlackoutClassSelectDisplayData UBlackoutClassSelectWidgetController::
BuildDisplayData(int32 Index) const
{
	FBlackoutClassSelectDisplayData Data;
	if (!Roster.IsValid() ||!Roster->Characters.IsValidIndex(Index))
	{
		return Data;
	}
	
	Data.CharacterData = Roster->Characters[Index];
	if (Data.CharacterData)
	{
		Data.PrimaryStat = LookupFirearmStat(Data.CharacterData->StartingPrimaryWeapon);
		Data.bPrimaryHasPelletCount = LookupShotgunPelletCount(
			Data.CharacterData->StartingPrimaryWeapon,
			Data.PrimaryPelletCount);
		Data.SecondaryStat=LookupFirearmStat(Data.CharacterData->StartingSecondaryWeapon);
		Data.bSecondaryHasPelletCount = LookupShotgunPelletCount(
			Data.CharacterData->StartingSecondaryWeapon,
			Data.SecondaryPelletCount);
	}
	return Data;
	
}

FBlackoutFirearmStat UBlackoutClassSelectWidgetController::LookupFirearmStat(
	TSubclassOf<ABOFirearm> WeaponClass) const
{
	if (!WeaponClass)
	{
		return{};
	}
	
	const ABOFirearm* CDO = WeaponClass.GetDefaultObject();
	if (!CDO)
	{
		return {};
	}
	const FDataTableRowHandle RowHandle = CDO->GetUIStatsRow();
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return {};
	}
	
	if (const FBlackoutFirearmStat* Row = RowHandle.GetRow<FBlackoutFirearmStat>(TEXT("ClassSelect LookupFirearmStat")))
	{
		return *Row;
	}
	return {};
}

bool UBlackoutClassSelectWidgetController::LookupShotgunPelletCount(
	TSubclassOf<ABOFirearm> WeaponClass, int32& OutPelletCount) const
{
	OutPelletCount = 0;
	if (!WeaponClass)
	{
		return false;
	}

	const ABOShotgunFirearm* ShotgunCDO = Cast<ABOShotgunFirearm>(WeaponClass.GetDefaultObject());
	if (!ShotgunCDO)
	{
		return false;
	}

	const FDataTableRowHandle RowHandle = ShotgunCDO->GetUIStatsRow();
	if (!RowHandle.DataTable || RowHandle.RowName.IsNone())
	{
		return false;
	}

	if (const FBlackoutShotgunFirearmStat* Row = RowHandle.GetRow<FBlackoutShotgunFirearmStat>(TEXT("ClassSelect LookupShotgunPelletCount")))
	{
		OutPelletCount = FMath::Max(Row->PelletCount, 1);
		return true;
	}

	BO_LOG_CORE(Warning, "ClassSelect LookupShotgunPelletCount: 산탄 스탯 행 조회 실패 Weapon=%s Row=%s",
		*GetNameSafe(WeaponClass.Get()),
		*RowHandle.RowName.ToString());
	return false;
}

void UBlackoutClassSelectWidgetController::UpdatePreviewPawn()
{
	
	if (!PreviewManager.IsValid())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsOfClass(this ,ABlackoutCharacterPreviewManager::StaticClass(),Found);
		if (Found.Num()>0)
		{
			PreviewManager= Cast<ABlackoutCharacterPreviewManager>(Found[0]);
		}
	}
	
	// 매니저가 없거나, 있어도 아직 BeginPlay 전이라 RT 미준비면 재시도.
	// GetAllActorsOfClass 는 BeginPlay 전 액터도 반환하므로(RT/CaptureComp null),
	// 액터 존재가 아니라 RT 준비 여부로 초기화 완료를 판정한다.
	// (스트리밍 서브레벨 L_CharacterPreview — 정원 1 싱글에선 UI 가 로드보다 먼저 열림.)
	UTextureRenderTarget2D* RT = PreviewManager.IsValid() ? PreviewManager->GetRenderTarget() : nullptr;
	if (!RT)
	{
		const UWorld* World = PlayerController.IsValid() ? PlayerController->GetWorld() : nullptr;
		if (World && PreviewManagerRetryCount < PreviewManagerMaxRetries)
		{
			++PreviewManagerRetryCount;
			World->GetTimerManager().SetTimer(PreviewManagerRetryHandle, this,
				&UBlackoutClassSelectWidgetController::UpdatePreviewPawn, 0.1f, false);
		}
		else
		{
			BO_LOG_CORE(Warning, "ClassSelect: PreviewManager RT 미준비 — 프리뷰 비활성 (재시도 %d회 초과)", PreviewManagerRetryCount);
		}
		return;
	}
	PreviewManagerRetryCount = 0;

	// RT 한 번만 broadcast — Widget 이 Image_Portrait 에 동적 bind
	if (!bRenderTargetBroadcast)
	{
		OnPreviewRenderTargetReady.Broadcast(RT);
		bRenderTargetBroadcast = true;
	}

	if (!Roster.IsValid() || !Roster->Characters.IsValidIndex(CurrentIndex))
	{
		return;
	}

	const UBOCharacterData* Selected= Roster->Characters[CurrentIndex];
	if (!Selected)
	{
		return;
	}
	PreviewManager->SetPreviewCharacter(Selected->PawnClass);
}
