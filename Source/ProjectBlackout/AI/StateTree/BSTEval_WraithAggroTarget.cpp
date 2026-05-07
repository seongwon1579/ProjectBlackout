#include "BSTEval_WraithAggroTarget.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void FBSTEval_WraithAggroTarget::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (!Data.Controller || !Data.Controller->HasAuthority())
	{
		return;
	}
	
	APawn* OwnerPawn =Data.Controller->GetPawn();
	if (!OwnerPawn)
	{
		return;
	}
	UWorld* World = OwnerPawn->GetWorld();
	
	if (!World)
	{
		return;
	}
	
	APawn* BestTarget = nullptr;
	float BestScore = -1.f;
	
	// 모든 플레이어 캐릭터 평가 (멀면 점수 0 )
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}
		
		ABlackoutPlayerCharacter* Player = Cast<ABlackoutPlayerCharacter>(PC->GetPawn());
		if (!Player)
		{
			continue;
		}
		
		const float Distance = FVector::Dist(OwnerPawn->GetActorLocation(), Player->GetActorLocation());
		
		const float DistanceScore = FMath::Max(0.0f , 1.0f -(Distance/ Data.MaxRange));
	
		// 합산 
		const float Score = Data.DistanceWeight *DistanceScore;
		
		// TODO: HP 낮을수록
		// TODO: 분산 (같은 타겟 회피)
		// TODO: 위협 누적
		
		if (Score >BestScore)
		{
			BestScore = Score;
			BestTarget = Player;
		}
	}
	Data.OutTarget = BestTarget;
}
