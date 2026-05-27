// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutClassSelectWidgetController.h"

#include "BlackoutLog.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Data/BOCharacterData.h"
#include "Data/BOCharacterRoster.h"
#include "Engine/DataTable.h"
#include "Framework/BlackoutPlayerController.h"
#include "Framework/BlackoutCharacterPreviewManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"

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
		Data.SecondaryStat=LookupFirearmStat(Data.CharacterData->StartingSecondaryWeapon);
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
	
	if (!PreviewManager.IsValid())
	{
		return;
	}

	// RT 한 번만 broadcast — Widget 이 Image_Portrait 에 동적 bind
	if (!bRenderTargetBroadcast)
	{
		if (UTextureRenderTarget2D* RT = PreviewManager->GetRenderTarget())
		{
			OnPreviewRenderTargetReady.Broadcast(RT);
			bRenderTargetBroadcast = true;
		}
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
