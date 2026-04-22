#pragma once

#include "CoreMinimal.h"
#include "BlackoutTypes.generated.h"

UENUM(BlueprintType)
enum class EBlackoutAbilityInputID : uint8
{
	None            UMETA(DisplayName = "None"),
	Confirm         UMETA(DisplayName = "Confirm"),
	Cancel          UMETA(DisplayName = "Cancel"),
	Fire            UMETA(DisplayName = "Fire"),
	Reload          UMETA(DisplayName = "Reload"),
	Dodge           UMETA(DisplayName = "Dodge"),
	Sprint          UMETA(DisplayName = "Sprint"),
	Melee           UMETA(DisplayName = "Melee"),
	UseRelic        UMETA(DisplayName = "UseRelic"),
	UseConsumable   UMETA(DisplayName = "UseConsumable"),
	Interact        UMETA(DisplayName = "Interact"),
};

UENUM(BlueprintType)
enum class EBattleTransitionType : uint8
{
	LobbyToBattle       UMETA(DisplayName = "Lobby To Battle"),
	CheckpointRest      UMETA(DisplayName = "Checkpoint Rest"),
	PartyWipeRestart    UMETA(DisplayName = "Party Wipe Restart"),
};

// 매치 전체 생애주기 상태. ABlackoutGameState::CurrentMatchState 에서 Replicated.
UENUM(BlueprintType)
enum class EBlackoutMatchState : uint8
{
	InLobby     UMETA(DisplayName = "In Lobby"),      // 로비 맵 진행 · Ready Check 대기
	Starting    UMETA(DisplayName = "Starting"),      // 전투 맵으로 ServerTravel 중
	InCombat    UMETA(DisplayName = "In Combat"),     // 전투 맵 진행
	Ended       UMETA(DisplayName = "Ended"),         // 승리/패배 후
};
