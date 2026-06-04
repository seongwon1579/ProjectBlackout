// Fill out your copyright notice in the Description page of Project Settings.


#include "BOHighlightComponent.h"

#include "GameFramework/Character.h"


// Sets default values for this component's properties
UBOHighlightComponent::UBOHighlightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UBOHighlightComponent::BeginPlay()
{
	Super::BeginPlay();
	
	ApplyStencil();
	
	// 본인 제외가 필요한 폰만 컨트를러 변경 추적 ( 적은 바인딩 X) 
	if (bSuppressForLocalViewer)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			OwnerPawn->ReceiveControllerChangedDelegate.AddDynamic(this, &UBOHighlightComponent::HandleControllerChanged);
		}
	}
}

void UBOHighlightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	
	if (bSuppressForLocalViewer)
	{
		if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
		{
			OwnerPawn->ReceiveControllerChangedDelegate.RemoveDynamic(this , &UBOHighlightComponent::HandleControllerChanged);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}



void UBOHighlightComponent::ApplyStencil()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}
	
	bool bSuppress = false;
	if (bSuppressForLocalViewer)
	{
		if (APawn* OwnerPawn = Cast<APawn>(OwnerActor))
		{
			bSuppress = OwnerPawn->IsLocallyControlled();
		}
	}
	
	const bool bEnable = (BaseStencil != EBlackoutStencil::None) && !bSuppress;
	const int32 StencilValue = static_cast<int32>(BaseStencil);

	TInlineComponentArray<UMeshComponent*> MeshComponents(OwnerActor);
	OwnerActor->GetComponents(MeshComponents);
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}
		MeshComponent->SetRenderCustomDepth(bEnable);
		if (bEnable)
		{
			MeshComponent->SetCustomDepthStencilValue(StencilValue);
		}
	}
}

void UBOHighlightComponent::HandleControllerChanged(APawn* Pawn,
	AController* OldController ,AController* NewController)
{
	ApplyStencil();
}



