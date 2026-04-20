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
