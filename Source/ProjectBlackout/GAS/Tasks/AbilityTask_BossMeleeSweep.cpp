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

	const FVector StartLoc = Mesh->GetSocketLocation(StartSocketName);
	const FVector EndLoc   = Mesh->GetSocketLocation(EndSocketName);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BossMeleeSweep), false, OwnerActor);
	for (const TWeakObjectPtr<AActor>& HitActor : HitActors)
	{
		if (HitActor.IsValid())
		{
			QueryParams.AddIgnoredActor(HitActor.Get());
		}
	}

	FHitResult HitResult;
	const bool bHit = OwnerActor->GetWorld()->SweepSingleByChannel(
		HitResult,
		StartLoc,
		EndLoc,
		FQuat::Identity,
		BlackoutCollisionChannels::WeaponTrace,
		FCollisionShape::MakeSphere(SweepRadius),
		QueryParams
	);

#if ENABLE_DRAW_DEBUG
	DrawDebugSweptSphere(
		OwnerActor->GetWorld(),
		StartLoc,
		EndLoc,
		SweepRadius,
		bHit ? FColor::Red : FColor::Green,
		false,
		2.f
	);
#endif

	if (bHit && HitResult.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());

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
