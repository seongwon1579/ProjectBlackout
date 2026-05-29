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
	SwapWeapon      UMETA(DisplayName = "SwapWeapon"),
	Dodge           UMETA(DisplayName = "Dodge"),
	Sprint          UMETA(DisplayName = "Sprint"),
	Melee           UMETA(DisplayName = "Melee"),
	UseRelic        UMETA(DisplayName = "UseRelic"),
	UseConsumable1   UMETA(DisplayName = "UseConsumable1"),
	UseConsumable2   UMETA(DisplayName = "UseConsumable2"),
	Interact        UMETA(DisplayName = "Interact"),
	Aim             UMETA(DisplayName = "Aim"),
};

UENUM(BlueprintType)
enum class EBattleTransitionType : uint8
{
	LobbyToBattle       UMETA(DisplayName = "Lobby To Battle"),
	CheckpointRest      UMETA(DisplayName = "Checkpoint Rest"),
	PartyWipeRestart    UMETA(DisplayName = "Party Wipe Restart"),
	SurrenderRestart    UMETA(DisplayName = "Surrender Restart"),
};

// 매치 전체 생애주기 상태. ABlackoutGameState::CurrentMatchState 에서 Replicated.
UENUM(BlueprintType)
enum class EBlackoutMatchState : uint8
{
	InLobby         UMETA(DisplayName = "In Lobby"),         // 레거시(로비 흐름) — 미사용, 로비 철거 시 제거
	Starting        UMETA(DisplayName = "Starting"),         // 레거시(ServerTravel) — 미사용, 로비 철거 시 제거
	InCombatReady   UMETA(DisplayName = "Combat Ready"),     // 레거시(체크포인트 Ready) — 미사용, 로비 철거 시 제거
	InCombat        UMETA(DisplayName = "In Combat"),        // 레거시(단일 전투) — 미사용, 로비 철거 시 제거
	Ended           UMETA(DisplayName = "Ended"),            // 승리/패배 후

	// 단일 맵 런-페이즈
	WaitingForPlayers UMETA(DisplayName = "Waiting For Players"), // 데디 접속 대기 (4인 미만)
	ShelterPrep       UMETA(DisplayName = "Shelter Prep"),        // 시작 쉘터: 클래스선택·Ready 대기
	MidBossCombat     UMETA(DisplayName = "Mid-Boss Combat"),     // Shrewd 활성
	MainBossCombat    UMETA(DisplayName = "Main-Boss Combat"),    // Ravager 활성
};

UENUM(BlueprintType)
enum class EBlackoutMatchEndReason:uint8
{
	BossDefeated UMETA(DisplayName = "Boss Defeated"),
	AllPlayersLeft UMETA(DisplayName = "All Players Left"),
	Timeout UMETA(DisplayName = "Timeout"),
	
};

// 보스 종류 , 매치 진행 스테이지와 OnBossDefeated 분기에 사용
UENUM(BlueprintType)
enum class EBossType : uint8
{
	Mid UMETA(DisplayName = "Mid Boss"),
	Main UMETA(DisplayName = "Main Boss"),
};
