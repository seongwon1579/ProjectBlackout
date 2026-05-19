#include "BODissolveComponent.h"

#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UBODissolveComponent::UBODissolveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UBODissolveComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeDissolve();
}

void UBODissolveComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DissolveTimeline.TickTimeline(DeltaTime);
}

void UBODissolveComponent::InitializeDissolve()
{
	// DMI 는 컴포넌트 수명 동안 1회만 생성 후 재사용 (풀 재사용 시 BeginPlay 미발화이므로 안전).
	if (DissolveDMIs.Num() > 0)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 소유 액터의 모든 MeshComponent(캐릭터 메쉬 + BP 무기 등) 슬롯을 DMI 화.
	// 특정 컴포넌트를 참조하지 않으므로 BP 구성에 컴포넌트가 의존하지 않는다.
	TArray<UMeshComponent*> MeshComps;
	Owner->GetComponents<UMeshComponent>(MeshComps);

	USceneComponent* NiagaraAttachParent = nullptr;
	for (UMeshComponent* MeshComp : MeshComps)
	{
		if (!MeshComp)
		{
			continue;
		}

		if (!NiagaraAttachParent && MeshComp->IsA<USkeletalMeshComponent>())
		{
			NiagaraAttachParent = MeshComp;
		}

		for (int32 Index = 0; Index < MeshComp->GetNumMaterials(); ++Index)
		{
			if (UMaterialInstanceDynamic* MID = MeshComp->CreateDynamicMaterialInstance(Index))
			{
				DissolveDMIs.Add(MID);
			}
		}
	}

	if (!NiagaraAttachParent)
	{
		NiagaraAttachParent = Owner->GetRootComponent();
	}

	if (DissolveNiagaraSystem && NiagaraAttachParent)
	{
		DissolveNiagara = NewObject<UNiagaraComponent>(Owner);
		DissolveNiagara->SetAutoActivate(false);
		DissolveNiagara->SetAsset(DissolveNiagaraSystem);
		DissolveNiagara->RegisterComponent();
		DissolveNiagara->AttachToComponent(
			NiagaraAttachParent, FAttachmentTransformRules::KeepRelativeTransform);
		DissolveNiagara->Deactivate();
	}

	if (DissolveCurve)
	{
		FOnTimelineFloat UpdateDelegate;
		UpdateDelegate.BindUFunction(this, FName(TEXT("UpdateDissolve")));
		DissolveTimeline.AddInterpFloat(DissolveCurve, UpdateDelegate);

		FOnTimelineEvent FinishedDelegate;
		FinishedDelegate.BindUFunction(this, FName(TEXT("OnDissolveFinished")));
		DissolveTimeline.SetTimelineFinishedFunc(FinishedDelegate);

		DissolveTimeline.SetTimelineLength(DissolveDuration);
		DissolveTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_TimelineLength);
	}
}

void UBODissolveComponent::PlayDissolve()
{
	if (bIsDissolving)
	{
		return;
	}

	Multicast_PlayDissolve();

	// 서버: 클라 Timeline 완료를 margin 으로 보장 후 완료 통지(풀 반환은 캐릭터 권위).
	AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			FTimerDelegate FinishedDelegate;
			FinishedDelegate.BindWeakLambda(this, [this]()
			{
				OnServerDissolveFinished.Broadcast();
			});
			World->GetTimerManager().SetTimer(
				PoolReturnTimerHandle, FinishedDelegate, DissolveDuration + PoolReturnMargin, false);
		}
	}
}

void UBODissolveComponent::ResetDissolve()
{
	bIsDissolving = false;
	DissolveTimeline.Stop();
	DissolveTimeline.SetNewTime(0.f);

	// 에셋 컨벤션: DissolveAmount 1 = 멀쩡, 0 = 완전 분해. 풀 재사용 시 1 로 복원.
	for (UMaterialInstanceDynamic* MID : DissolveDMIs)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(DissolveAmountParam, 1.f);
		}
	}

	if (DissolveNiagara)
	{
		DissolveNiagara->SetVariableFloat(NiagaraAmountParam, 1.f);
		DissolveNiagara->ResetSystem();   // 이전 죽음의 잔존 파티클 제거
		DissolveNiagara->Deactivate();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PoolReturnTimerHandle);
	}
}

void UBODissolveComponent::Multicast_PlayDissolve_Implementation()
{
	bIsDissolving = true;

	// 데디 서버는 시각 요소 불필요 — 완료 통지는 PlayDissolve 의 서버 타이머가 처리.
	if (GetOwner() && GetOwner()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (DissolveNiagara)
	{
		DissolveNiagara->Activate(true);
	}
	DissolveTimeline.PlayFromStart();
}

void UBODissolveComponent::UpdateDissolve(float Value)
{
	for (UMaterialInstanceDynamic* MID : DissolveDMIs)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(DissolveAmountParam, Value);
		}
	}

	if (DissolveNiagara)
	{
		DissolveNiagara->SetVariableFloat(NiagaraAmountParam, Value);
	}
}

void UBODissolveComponent::OnDissolveFinished()
{
	// 클라 시각 종료만. 풀 반환은 서버 타이머 → 캐릭터 권위.
	if (DissolveNiagara)
	{
		DissolveNiagara->Deactivate();
	}
}
