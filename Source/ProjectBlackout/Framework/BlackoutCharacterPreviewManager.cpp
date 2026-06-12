// Fill out your copyright notice in the Description page of Project Settings.


#include "BlackoutCharacterPreviewManager.h"
#include "BlackoutLog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/SceneCapture2D.h"
#include "Camera/CameraComponent.h"
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
	
	
	CaptureComp = FindComponentByClass<USceneCaptureComponent2D>();
	if (CaptureComp)
	{
		CaptureComp->TextureTarget = DynamicRT;
		CaptureComp->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

		SetPreviewCaptureActive(false);
	}
	BO_LOG_CORE(Log, "Manager BeginPlay: DynamicRT created (%dx%d), CaptureComp=%s",
		RTSizeX, RTSizeY, CaptureComp ? TEXT("OK") : TEXT("NULL"));
	
}

// Sets default values
ABlackoutCharacterPreviewManager::ABlackoutCharacterPreviewManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	SpawnRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnRoot"));
	RootComponent = SpawnRoot;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpawnRoot);
	ViewCamera->SetRelativeLocation(FVector(-300.f, 0.f, 120.f));
	ViewCamera->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
}

void ABlackoutCharacterPreviewManager::SetPreviewCharacter(TSubclassOf<AActor> PreviewClass)
{
	if (CurrentPreviewClass == PreviewClass)
	{
		return; // 같은 클래스는 재스폰 X
	}

	ClearPreview();

	if (!PreviewClass)
	{
		return;
	}

	FTransform SpawnTransform = GetActorTransform();
	SpawnTransform.SetRotation(FRotator(0.f, PreviewYawOffset, 0.f).Quaternion());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentPreviewActor  = GetWorld()->SpawnActor<AActor>(PreviewClass, SpawnTransform, SpawnParams);
	if (CurrentPreviewActor)
	{
		// Preview Pawn 은 각 client local — server 의 Pawn 이 다른 client 로 replicate 되면 stack
		CurrentPreviewActor->SetReplicates(false);

		if (CaptureComp)
		{
			CaptureComp->ShowOnlyActors.Reset();
			CaptureComp->ShowOnlyActors.Add(CurrentPreviewActor);

			TArray<AActor*> AttachedActors;
			CurrentPreviewActor->GetAttachedActors(AttachedActors);
			for (AActor* Attached : AttachedActors)
			{
				CaptureComp->ShowOnlyActors.Add(Attached);
			}
		}
	}
	CurrentPreviewClass = PreviewClass;

	BO_LOG_CORE(Log, "CharacterPreview spawn: %s @ %s", *GetNameSafe(PreviewClass.Get()), *SpawnTransform.GetLocation().ToString());
}

void ABlackoutCharacterPreviewManager::ClearPreview()
{
	if (CurrentPreviewActor)
	{
		CurrentPreviewActor->Destroy();
		CurrentPreviewActor = nullptr;
	}

	CurrentPreviewClass = nullptr;

	if (CaptureComp)
	{
		CaptureComp->ShowOnlyActors.Reset();
	}
}

void ABlackoutCharacterPreviewManager::SetPreviewCaptureActive(bool bActive)
{
	if (!CaptureComp)
	{
		return;
	}

	// UI 가 닫힌 동안에는 렌더타깃 갱신 자체를 멈추고, 열렸을 때만 애니메이션을 매 프레임 캡처한다.
	CaptureComp->bCaptureEveryFrame = bActive;
	CaptureComp->bCaptureOnMovement = false;
	CaptureComp->SetVisibility(bActive);
	CaptureComp->SetComponentTickEnabled(bActive);

	if (bActive)
	{
		CaptureComp->Activate(true);
	}
	else
	{
		CaptureComp->Deactivate();
	}
}
