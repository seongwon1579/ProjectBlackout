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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Reloading);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_UseConsumable);
	
	// ─── Character Abilities ───────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_SwapWeapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_UseConsumable);

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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Consumable_HealAmount);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Consumable_Duration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Consumable_StaminaCostMultiplier);

	// ─── Consumables ──────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_BloodRoot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Consumable_GulSerum);

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

	// ─── Reload Animation ─────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Fire_RustyLeverAction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Reload_ChicagoTypewriter);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Reload_RepeaterPistol);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Reload_Sporebloom);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Reload_Meridian);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Reload_RustyLeverAction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Reload_DoubleBarrel);

	// ─── Lobby ─────────────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(LobbyTag_InfiniteAmmo);

	// ─── Gameplay Cues ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Character_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_MeridianGrenade_Explosion);
	
	// ─── Ravager Abilities ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Bite_Single);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Bite_DoubleFromSingle);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Swipe_RFromSingle);
	
	
	// ─── Wraith Abilities ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_FireTwinArrows);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Wraith_FireTwinArrows);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Wraith_FireTwinArrows_Shot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_Teleport);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Wraith_Teleport);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_BowShove);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Wraith_BowShove);
	
	// ─── Wraith Gameplay Cues ───────────────────────────────────────────────────────── 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Teleport_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Teleport_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Wraith_Teleport_Vanish);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Wraith_Teleport_Appear);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_BowShove);
	

	// ─── Attack Events ────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_SweepStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_SweepEnd);

	// ─── Montage Events ───────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ConsumableApply);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_FireWeaponStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ReloadWeaponStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ReloadAmmoCommit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_WeaponSwapCommit);
}
