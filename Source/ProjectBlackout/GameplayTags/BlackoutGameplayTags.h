#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace BlackoutGameplayTags
{
	// ─── Character State ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Downed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Reviving);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BeingRevived);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Invulnerable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_MovementLocked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Locked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Stunned);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_StunBroken);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Aiming);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Sprinting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Reloading);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_UseConsumable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_UseRelic);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Pulled);
	
	// ─── Character Abilities ───────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Dodge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Melee);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Aim);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Reload);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_SwapWeapon);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_UseRelic);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Player_UseConsumable);

	// ─── Character Class ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Class_Assault);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Class_Demolition);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Class_Sniper);

	// ─── GAS SetByCaller 데이터 키 ────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stun);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MaxHealth);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MaxStamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Stamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_MovementSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_DamageNumber_PredictedOnly);
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

	// ─── Impact Surface ───────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Surface_Stone);

	// ─── Shoot Animation ─────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Fire_RustyLeverAction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Fire_Sporebloom);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Fire_ChicagoTypewriter);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Animation_Fire_SecondaryCommon);
	
	// ─── Reload Animation ─────────────────────────────────────────────────────
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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Character_Revive);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Consumable_Use);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Consumable_BloodRoot_Use);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Consumable_GulSerum_Use);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Relic_Use);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Default_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Default_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Default_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Default_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Default_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Default_Impact_Stone);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Impact_Stone);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Reload_In);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_ChicagoTypewriter_Reload_Out);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RepeaterPistol_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RepeaterPistol_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RepeaterPistol_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RepeaterPistol_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RepeaterPistol_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RepeaterPistol_Impact_Stone);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RepeaterPistol_Reload);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Impact_Stone);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Reload_Rack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Reload_Release);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Sporebloom_Reload_Reload);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Impact_Stone);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Explosion);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Reload_In);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Meridian_Reload_Out);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Impact_Stone);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Bolt_Push);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Bolt_Pull);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Reload_Mag_In);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Reload_Mag_Out);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Reload_Bolt_Push);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_RustyLeverAction_Reload_Bolt_Pull);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Impact_Stone);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Reload_Open);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Reload_Insert);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_DoubleBarrel_Reload_Close);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_SteelSword_Swing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_SteelSword_Impact_Default);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_SteelSword_Impact_Flesh);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_SteelSword_Impact_Metal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_SteelSword_Impact_Stone);
	
	// ─── Ravager Abilities ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_PhaseLock);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_TargetChange);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Bite_Single);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_FromSingle_SwipeL);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_FromSingle_SwipeR);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_FromSingle_Double);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Swipe_L);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Swipe_LFromR);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Swipe_R);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Swipe_RFromL);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_FlashKick);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Turn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Turn_L_90);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Turn_L_135);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Turn_L_180);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Turn_R_90);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Turn_R_135);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Turn_R_180);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Evade_L);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Evade_R);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Shockwave);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_SpawnMinion);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Gorenado);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_EnergyBurst);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Charged);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ravager_Howl);
	
	// ─── Ravager Gameplay Cues ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Ravager_Gorenado);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Ravager_EnergyBurst);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Ravager_Shockwave_Launch);
	
	// ─── Hollow Abilities ──────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Hollow_Spawn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Hollow_PreRoll);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Hollow_Attack);
	
	// ─── Shrewd Abilities ──────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Shrewd_Teleport_ToPoint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Shrewd_Teleport_ByEQS);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Shrewd_Teleport);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Shrewd_Fire_Arrow_Straight);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Shrewd_Fire_Arrow_Explosive);
	
	// ─── Shrewd Gameplay Cues ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Shrewd_Projectile_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Shrewd_Projectile_Explosion);

	// ─── Wraith Abilities ─────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_FireTwinArrows);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Wraith_FireTwinArrows);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Wraith_FireTwinArrows_Shot);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_Teleport);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Wraith_Teleport);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Wraith_BowShove);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Wraith_BowShove);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Wraith_Teleport_Vanish);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Wraith_Teleport_Appear);
	
	// ─── Wraith Gameplay Cues ───────────────────────────────────────────────────────── 
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Fire);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Teleport_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Teleport_End);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_BowShove);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Projectile_Trail);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Wraith_Projectile_Explosion);
	
	// ─── Attack Events ────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_OnCollision);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_OffCollision);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_OnGorenado);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_OffGorenado);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_Spawn_Projectile);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_Charge_LoopStart);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_OnEnergyBurst);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_OffEnergyBurst);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_KickFlash_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Attack_KickFlash_End);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Enemy_Shrewd_Attack_FireArrow);
	
	// ─── Montage Events ───────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ConsumableApply);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_RelicApply);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_FireWeaponStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ReloadWeaponStart);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_ReloadAmmoCommit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_WeaponSwapCommit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_AbilityCancelable);

	// ─── Shelter ──────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_InShelter);          // 쉘터존 안에 있는 동안 부여
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_ShelterScoped);     // 쉘터 내 적용된 GE 표식 (이탈 시 strip)
}
