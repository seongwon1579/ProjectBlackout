#include "Items/BlackoutConsumablePickup.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/BlackoutLog.h"
#include "Data/BOConsumableData.h"
#include "Framework/BlackoutPlayerState.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GameFramework/Pawn.h"

ABlackoutConsumablePickup::ABlackoutConsumablePickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	SetRootComponent(InteractionSphere);
	InteractionSphere->InitSphereRadius(80.0f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(InteractionSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

bool ABlackoutConsumablePickup::CanInteract_Implementation(AActor* Interactor) const
{
	if (bIsCollected || !ConsumableData || !ConsumableData->ConsumableTag.IsValid() || PickupAmount <= 0)
	{
		return false;
	}

	ABlackoutPlayerState* PlayerState = nullptr;
	int32 CurrentCount = 0;
	int32 MaxCount = 0;
	return TryResolvePickupState(Interactor, PlayerState, CurrentCount, MaxCount) && CurrentCount < MaxCount;
}

void ABlackoutConsumablePickup::OnInteract_Implementation(AActor* Interactor)
{
	if (!HasAuthority())
	{
		BO_LOG_CORE(Warning, "소모품 픽업 실패: 서버가 아닌 곳에서 OnInteract가 호출되었습니다. Pickup=%s", *GetName());
		return;
	}

	if (bIsCollected)
	{
		return;
	}

	ABlackoutPlayerState* PlayerState = nullptr;
	int32 CurrentCount = 0;
	int32 MaxCount = 0;
	if (!TryResolvePickupState(Interactor, PlayerState, CurrentCount, MaxCount))
	{
		BO_LOG_CORE(Warning, "소모품 픽업 실패: 플레이어 상태 또는 소모품 데이터가 유효하지 않습니다. Pickup=%s", *GetName());
		return;
	}

	if (CurrentCount >= MaxCount)
	{
		BO_LOG_CORE(Verbose,
			"소모품 픽업 무시: 이미 최대 수량입니다. Pickup=%s Player=%s Tag=%s Current=%d Max=%d",
			*GetName(),
			*GetNameSafe(PlayerState),
			*ConsumableData->ConsumableTag.ToString(),
			CurrentCount,
			MaxCount);
		return;
	}

	const int32 NewCount = FMath::Clamp(CurrentCount + PickupAmount, 0, MaxCount);
	if (!PlayerState->SetConsumableCount(ConsumableData->ConsumableTag, NewCount))
	{
		BO_LOG_CORE(Warning,
			"소모품 픽업 실패: 수량 반영에 실패했습니다. Pickup=%s Player=%s Tag=%s",
			*GetName(),
			*GetNameSafe(PlayerState),
			*ConsumableData->ConsumableTag.ToString());
		return;
	}

	bIsCollected = true;
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);

	BO_LOG_CORE(Log,
		"소모품 픽업 성공: Pickup=%s Player=%s Tag=%s +%d (%d -> %d)",
		*GetName(),
		*GetNameSafe(PlayerState),
		*ConsumableData->ConsumableTag.ToString(),
		PickupAmount,
		CurrentCount,
		NewCount);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
}

FText ABlackoutConsumablePickup::GetInteractionPrompt_Implementation() const
{
	// 픽업 1차 구현은 E 아이콘만 사용하므로 별도 문구를 노출하지 않습니다.
	return FText::GetEmpty();
}

bool ABlackoutConsumablePickup::TryResolvePickupState(
	AActor* Interactor,
	ABlackoutPlayerState*& OutPlayerState,
	int32& OutCurrentCount,
	int32& OutMaxCount) const
{
	OutPlayerState = nullptr;
	OutCurrentCount = 0;
	OutMaxCount = 0;

	if (!Interactor || !ConsumableData || !ConsumableData->ConsumableTag.IsValid())
	{
		return false;
	}

	const APawn* InteractorPawn = Cast<APawn>(Interactor);
	OutPlayerState = InteractorPawn ? InteractorPawn->GetPlayerState<ABlackoutPlayerState>() : nullptr;
	if (!OutPlayerState)
	{
		return false;
	}

	OutCurrentCount = OutPlayerState->GetConsumableCount(ConsumableData->ConsumableTag);
	OutMaxCount = FMath::Max(0, ConsumableData->MaxCount);
	return OutMaxCount > 0;
}
