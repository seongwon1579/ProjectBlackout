#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace BlackoutGameplayTags
{
	// ─── Character State ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Downed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invulnerable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Locked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Aiming);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Sprinting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);

	// ─── Character Class ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Class_Assault);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Class_Demolition);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Class_Sniper);

	// ─── GAS SetByCaller 데이터 키 ────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MaxStamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MovementSpeed);

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

	// ─── Weapon Slot ───────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Primary);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_Secondary);

	// ─── Lobby ─────────────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LobbyTag_InfiniteAmmo);

	// ─── Gameplay Cues ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Character_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_MeridianGrenade_Explosion);
	
	// ─── Ravager Abilities ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Enemy_Attack1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Enemy_Attack2);
	
	// ─── Wraith Abilities ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_FireTwinArrows);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_Teleport);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_BowShove);
	
	// ─── Wraith Gameplay Cues ───────────────────────────────────────────────────────── 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Teleport_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Teleport_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_BowShove);
	
	// ─── Wraith State ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Wraith_Invulnerable);

	// ─── Attack Events ────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_SweepStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_SweepEnd);
}
