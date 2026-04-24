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
	InLobby         UMETA(DisplayName = "In Lobby"),         // 로비 맵 · 캐릭터 선택 · Ready Check
	Starting        UMETA(DisplayName = "Starting"),         // 전투 맵으로 ServerTravel 중
	InCombatReady   UMETA(DisplayName = "Combat Ready"),     // 전투 맵 도착 or 전멸 후 · 체크포인트 방 Ready 대기
	InCombat        UMETA(DisplayName = "In Combat"),        // 보스 활성 · 전투 진행
	Ended           UMETA(DisplayName = "Ended"),            // 승리/패배 후
};

UENUM(BlueprintType)
enum class EBlackoutMatchEndReason:uint8
{
	BossDefeated UMETA(DisplayName = "Boss Defeated"),
	AllPlayersLeft UMETA(DisplayName = "All Players Left"),
	Timeout UMETA(DisplayName = "Timeout"),
	
};
