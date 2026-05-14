#include "GAS/Tasks/AbilityTask_BossMeleeSweep.h"

#include "Core/BlackoutCollisionChannels.h"
#include "DrawDebugHelpers.h"
#include "KismetTraceUtils.h"
#include "GameFramework/Character.h"
#include "Interfaces/BlackoutDamageable.h"

UAbilityTask_BossMeleeSweep* UAbilityTask_BossMeleeSweep::CreateSweepTask(
	UGameplayAbility* OwningAbility,
	FName InStartSocketName,
	FName InEndSocketName,
	float InSweepRadius,
	UMeshComponent* InMeshOverride)
{
	UAbilityTask_BossMeleeSweep* Task = NewAbilityTask<UAbilityTask_BossMeleeSweep>(OwningAbility);
	Task->bTickingTask = true;
	Task->StartSocketName = InStartSocketName;
	Task->EndSocketName = InEndSocketName;
	Task->SweepRadius = InSweepRadius;
	Task->MeshOverride = InMeshOverride;
	return Task;
}

void UAbilityTask_BossMeleeSweep::Activate()
{
	Super::Activate();
	HitActors.Empty();
	bFirstTick = true;
}

void UAbilityTask_BossMeleeSweep::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	AActor* OwnerActor = GetOwnerActor();
	if (!OwnerActor)
	{
		EndTask();
		return;
	}

	UMeshComponent* Mesh = GetOwnerMesh();
	if (!Mesh || !Mesh->DoesSocketExist(StartSocketName) || !Mesh->DoesSocketExist(EndSocketName))
	{
		return;
	}

	const FVector CurStartLoc = Mesh->GetSocketLocation(StartSocketName);
	const FVector CurEndLoc   = Mesh->GetSocketLocation(EndSocketName);

	// 첫 틱에는 이전 위치를 현재와 동일하게 초기화
	if (bFirstTick)
	{
		PrevStartLoc = CurStartLoc;
		PrevEndLoc   = CurEndLoc;
		bFirstTick   = false;
	}

	const FVector ActorLoc = OwnerActor->GetActorLocation();
	const FVector MeshLoc  = Mesh->GetComponentLocation();

#if ENABLE_DRAW_DEBUG
	DrawDebugSphere(OwnerActor->GetWorld(), ActorLoc, 20.f, 8, FColor::Yellow, false, 0.f);
	DrawDebugSphere(OwnerActor->GetWorld(), MeshLoc, 15.f, 8, FColor::Blue, false, 0.f);
#endif

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossMeleeSweep), false, OwnerActor);
	for (const TWeakObjectPtr<AActor>& HitActor : HitActors)
	{
		if (HitActor.IsValid())
		{
			QueryParams.AddIgnoredActor(HitActor.Get());
		}
	}

	// 무기 축을 SweepSamples 등분한 각 지점을 이전→현재 경로로 스윕
	// → 소켓 두 끝만 체크할 때 중간 구간을 놓치는 tunneling 방지
	FHitResult HitResult;
	for (int32 i = 0; i < SweepSamples; i++)
	{
		const float T       = (float)i / (SweepSamples - 1);
		const FVector From  = FMath::Lerp(PrevStartLoc, PrevEndLoc, T);
		const FVector To    = FMath::Lerp(CurStartLoc,  CurEndLoc,  T);

#if ENABLE_DRAW_DEBUG
		DrawDebugSweptSphere(OwnerActor->GetWorld(), From, To, SweepRadius, FColor::Green, false, 0.1f);
#endif

		if (DoSweep(From, To, QueryParams, HitResult) && HitResult.GetActor())
		{
			break;
		}
	}
	const bool bHit = HitResult.GetActor() != nullptr;

#if ENABLE_DRAW_DEBUG
	if (bHit)
	{
		DrawDebugSphere(OwnerActor->GetWorld(), HitResult.ImpactPoint, SweepRadius, 12, FColor::Red, false, 0.1f);
	}
#endif

	PrevStartLoc = CurStartLoc;
	PrevEndLoc   = CurEndLoc;

	if (bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
#if ENABLE_DRAW_DEBUG
		DrawDebugSphere(OwnerActor->GetWorld(), HitResult.ImpactPoint, SweepRadius, 12, FColor::Orange, false, 1.f);
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *HitResult.GetActor()->GetName()));
#endif

		HitActors.Add(HitResult.GetActor());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnHit.Broadcast(HitResult);
		}
	}
}

void UAbilityTask_BossMeleeSweep::OnDestroy(bool bInOwnerFinished)
{
	HitActors.Empty();
	Super::OnDestroy(bInOwnerFinished);
}

UMeshComponent* UAbilityTask_BossMeleeSweep::GetOwnerMesh() const
{
	if (MeshOverride.IsValid())
	{
		return MeshOverride.Get();
	}
	if (const ACharacter* Character = Cast<ACharacter>(GetOwnerActor()))
	{
		return Character->GetMesh();
	}
	return nullptr;
}

bool UAbilityTask_BossMeleeSweep::DoSweep(const FVector& From, const FVector& To, const FCollisionQueryParams& Params, FHitResult& OutHit) const
{
	const AActor* Owner = GetOwnerActor();
	if (!Owner) return false;

	return Owner->GetWorld()->SweepSingleByChannel(
		OutHit,
		From,
		To,
		FQuat::Identity,
		BlackoutCollisionChannels::WeaponTrace,
		FCollisionShape::MakeSphere(SweepRadius),
		Params
	);
}
