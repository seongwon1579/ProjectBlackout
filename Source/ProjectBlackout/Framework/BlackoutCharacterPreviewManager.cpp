// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutCharacterPreviewManager.h"
#include "BlackoutLog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SceneCapture2D.h"
#include "Components/SceneCaptureComponent2D.h"


void ABlackoutCharacterPreviewManager::BeginPlay()
{
	Super::BeginPlay();
	
	// client 별 RT
	DynamicRT = NewObject<UTextureRenderTarget2D>(this);
	DynamicRT->RenderTargetFormat = RTF_RGBA16f;
	DynamicRT ->ClearColor = FLinearColor::Black;
	DynamicRT->InitAutoFormat(RTSizeX , RTSizeY );
	DynamicRT->UpdateResourceImmediate(true);
	
	TArray<AActor*> Captures;
	UGameplayStatics::GetAllActorsOfClass(this , ASceneCapture2D::StaticClass(), Captures);
	for (AActor* Actor : Captures)
	{
		if (ASceneCapture2D* SC  =Cast<ASceneCapture2D>(Actor))
		{
			SC->GetCaptureComponent2D()->TextureTarget = DynamicRT;
		}
	}
	BO_LOG_CORE(Log, "Manager BeginPlay: DynamicRT created (%dx%d), SceneCapture count=%d",
	RTSizeX, RTSizeY, Captures.Num());
	
}

// Sets default values
ABlackoutCharacterPreviewManager::ABlackoutCharacterPreviewManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SpawnRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnRoot"));
	RootComponent = SpawnRoot;
}

void ABlackoutCharacterPreviewManager::SetPreviewCharacter(
	TSubclassOf<APawn> PawnClass)
{
	
	if (CurrentPawnClass == PawnClass)
	{
		return; // 같은 클래스는 재스폰 X
	}
	
	ClearPreview();
	
	if (!PawnClass)
	{
		return;
	}
	
	FTransform SpawnTransform = GetActorTransform();
	SpawnTransform.SetRotation(FRotator(0.f ,PreviewYawOffset , 0.f).Quaternion());
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	CurrentPawn = GetWorld() ->SpawnActor<APawn>(PawnClass , SpawnTransform , SpawnParams);
	if (CurrentPawn)
	{
		// Preview Pawn 은 각 client local — server 의 Pawn 이 다른 client 로 replicate 되면 stack
		CurrentPawn->SetReplicates(false);
	}
	CurrentPawnClass = PawnClass;
	
	BO_LOG_CORE(Log, "CharacterPreview spawn: %s @ %s", *GetNameSafe(PawnClass.Get()), *SpawnTransform.GetLocation().ToString());
}

void ABlackoutCharacterPreviewManager::ClearPreview()
{
	if (CurrentPawn)
	{
		const FString PawnName = GetNameSafe(CurrentPawn);
		AController* C = CurrentPawn->GetController();

		const bool bDestroyed = CurrentPawn->Destroy();

		BO_LOG_CORE(Log, "ClearPreview: %s, ControllerWas=%s, DestroyResult=%d",
			*PawnName, *GetNameSafe(C), bDestroyed);
		CurrentPawn = nullptr;
	}

	CurrentPawnClass = nullptr;

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(this, APawn::StaticClass(), AllPawns);
	BO_LOG_CORE(Log, "Pawns in world after Clear: %d", AllPawns.Num());
}




