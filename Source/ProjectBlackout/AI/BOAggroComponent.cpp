#include "AI/BOAggroComponent.h"
#include "Data/BOBossData.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"

UBOAggroComponent::UBOAggroComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBOAggroComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !BossData) return;

	const float Decay = BossData->AggroDecayRate * DeltaTime;
	for (auto& Pair : DamageAccumulator)
	{
		Pair.Value = FMath::Max(0.f, Pair.Value - Pair.Value * Decay);
	}
}

APawn* UBOAggroComponent::GetHighestAggroTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return nullptr;

	APlayerState* BestPS   = nullptr;
	float          BestVal  = 0.f;
	APlayerState* SecondPS = nullptr;
	float          SecondVal = 0.f;

	for (const auto& Pair : DamageAccumulator)
	{
		if (!Pair.Key.IsValid()) continue;

		if (Pair.Value > BestVal)
		{
			SecondPS  = BestPS;
			SecondVal = BestVal;
			BestPS    = Pair.Key.Get();
			BestVal   = Pair.Value;
		}
		else if (Pair.Value > SecondVal)
		{
			SecondPS  = Pair.Key.Get();
			SecondVal = Pair.Value;
		}
	}

	if (!BestPS) return CurrentTarget.Get();

	// 1순위와 2순위 격차가 임계값 미만이면 현재 타겟 유지 (핑퐁 방지)
	if (BossData && SecondPS)
	{
		const float Gap = (BestVal - SecondVal) / FMath::Max(BestVal, 1.f);
		if (Gap < BossData->AggroDamageThreshold && CurrentTarget.IsValid())
		{
			return CurrentTarget.Get();
		}
	}

	// 타겟 전환 쿨다운 체크
	const float Now = GetWorld()->GetTimeSeconds();
	if (BossData && CurrentTarget.IsValid())
	{
		APawn* BestPawn = BestPS->GetPawn();
		if (BestPawn != CurrentTarget.Get() &&
			(Now - LastSwitchTime) < BossData->AggroSwitchCooldown)
		{
			return CurrentTarget.Get();
		}
	}

	APawn* BestPawn = BestPS->GetPawn();
	if (BestPawn && BestPawn != CurrentTarget.Get())
	{
		CurrentTarget  = BestPawn;
		LastSwitchTime = Now;
	}

	return CurrentTarget.Get();
}

void UBOAggroComponent::AddThreat(APawn* Source, float Amount)
{
	if (!Source) return;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority()) return;

	APlayerState* PS = Source->GetPlayerState();
	if (!PS) return;

	DamageAccumulator.FindOrAdd(PS) += Amount;
}
