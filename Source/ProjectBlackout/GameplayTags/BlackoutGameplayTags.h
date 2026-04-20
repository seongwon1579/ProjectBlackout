#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace BlackoutGameplayTags
{
	// ─── Character State ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Downed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invulnerable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Locked);

	// ─── GAS SetByCaller 데이터 키 ────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);

	// ─── Hitbox Parts ──────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Body_WeakSpot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Body_ArmoredLimb);

	// ─── Kill Conditions (조건부 자원 보상 판정) ──────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Kill_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Kill_WeakSpot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Kill_MultiTarget_Count3);

	// ─── Checkpoints ───────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Checkpoint_MidBoss);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Checkpoint_MainBoss);

	// ─── Lobby ─────────────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LobbyTag_InfiniteAmmo);
}
