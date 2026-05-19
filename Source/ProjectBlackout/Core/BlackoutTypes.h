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
};

// 매치 전체 생애주기 상태. ABlackoutGameState::CurrentMatchState 에서 Replicated.
UENUM(BlueprintType)
enum class EBlackoutMatchState : uint8
{
	InLobby         UMETA(DisplayName = "In Lobby"),         // [deprecated · Step3 제거] 구 로비 흐름
	Starting        UMETA(DisplayName = "Starting"),         // [deprecated · Step3 제거] 구 ServerTravel
	InCombatReady   UMETA(DisplayName = "Combat Ready"),     // [deprecated · Step3 제거] 구 체크포인트 Ready
	InCombat        UMETA(DisplayName = "In Combat"),        // [deprecated · Step3 제거] 구 전투
	Ended           UMETA(DisplayName = "Ended"),            // 승리/패배 후 (유지)

	// 단일맵 런-페이즈 (Step1 가산, Step3 에서 위 deprecated 대체)
	WaitingForPlayers UMETA(DisplayName = "Waiting For Players"), // 데디 접속 대기 (4인 미만)
	ShelterPrep       UMETA(DisplayName = "Shelter Prep"),        // 시작 쉘터: 클래스선택·Ready 대기
	MidBossCombat     UMETA(DisplayName = "Mid-Boss Combat"),     // Shrewd 활성
	ShelterMid        UMETA(DisplayName = "Shelter Mid"),         // 중간 거점: 휴식·재선택·Ready
	MainBossCombat    UMETA(DisplayName = "Main-Boss Combat"),    // Ravager 활성
};

UENUM(BlueprintType)
enum class EBlackoutMatchEndReason:uint8
{
	BossDefeated UMETA(DisplayName = "Boss Defeated"),
	AllPlayersLeft UMETA(DisplayName = "All Players Left"),
	Timeout UMETA(DisplayName = "Timeout"),
	
};
