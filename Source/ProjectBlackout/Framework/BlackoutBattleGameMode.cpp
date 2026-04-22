#include "BlackoutBattleGameMode.h"
#include "BlackoutPlayerState.h"
#include "BlackoutLog.h"
#include "Core/BlackoutTypes.h"
#include "GameFramework/PlayerController.h"

// 플레이어 접속 직후 전투 진입 자원 정책 적용.
void ABlackoutBattleGameMode::OnPlayerJoined(APlayerController* NewPlayer)
{
	if (!NewPlayer) { return; }

	if (ABlackoutPlayerState* PS = NewPlayer->GetPlayerState<ABlackoutPlayerState>())
	{
		PS->ApplyBattleTransitionPolicy(EBattleTransitionType::LobbyToBattle);
	}
}

// 화톳불 상호작용 시 호출되어 현재 체크포인트 액터 갱신.
void ABlackoutBattleGameMode::HandleCheckpoint(AActor* BonfireActor)
{
	if (!BonfireActor) { return; }

	CurrentCheckpointActor = BonfireActor;
	BO_LOG_NET(Log, "체크포인트 갱신: %s", *BonfireActor->GetName());
}

// 파티 전멸 감지 시 호출. 실제 복귀 로직은 체크포인트 액터 구현 이후 보강.
void ABlackoutBattleGameMode::HandlePartyWipe()
{
	Super::HandlePartyWipe();

	const FString Location = CurrentCheckpointActor
		? CurrentCheckpointActor->GetName()
		: TEXT("none");
	BO_LOG_NET(Log, "파티 전멸 - 체크포인트 복귀 대기 (CurrentCheckpoint=%s)", *Location);
}
