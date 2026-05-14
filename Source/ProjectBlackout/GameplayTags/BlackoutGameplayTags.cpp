#include "BlackoutGameplayTags.h"

namespace BlackoutGameplayTags
{
	// ─── Character State ───────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Downed,        "State.Downed");
	UE_DEFINE_GAMEPLAY_TAG(State_Invulnerable,  "State.Invulnerable");
	UE_DEFINE_GAMEPLAY_TAG(State_Locked,        "State.Locked");
	UE_DEFINE_GAMEPLAY_TAG(State_Aiming,        "State.Aiming");
	UE_DEFINE_GAMEPLAY_TAG(State_Sprinting,     "State.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking,     "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Reloading,     "State.Reloading");
	UE_DEFINE_GAMEPLAY_TAG(State_UseConsumable, "State.UseConsumable");
	UE_DEFINE_GAMEPLAY_TAG(State_UseRelic,      "State.UseRelic");
	
	// ─── Character Abilities ───────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Dodge,			"Ability.Player.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Aim,				"Ability.Player.Aim");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Reload,			"Ability.Player.Reload");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_SwapWeapon,		"Ability.Player.SwapWeapon");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Sprint,			"Ability.Player.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_UseRelic,			"Ability.Player.UseRelic");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_UseConsumable,	"Ability.Player.UseConsumable");

	// ─── Character Class ───────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Character_Class_Assault,    "Character.Class.Assault");
	UE_DEFINE_GAMEPLAY_TAG(Character_Class_Demolition, "Character.Class.Demolition");
	UE_DEFINE_GAMEPLAY_TAG(Character_Class_Sniper,     "Character.Class.Sniper");

	// ─── GAS SetByCaller 데이터 키 ────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage,        "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_MaxHealth,     "Data.Stat.MaxHealth");
	UE_DEFINE_GAMEPLAY_TAG(Data_Health,        "Data.Stat.Health");
	UE_DEFINE_GAMEPLAY_TAG(Data_MaxStamina,    "Data.Stat.MaxStamina");
	UE_DEFINE_GAMEPLAY_TAG(Data_Stamina,       "Data.Stat.Stamina");
	UE_DEFINE_GAMEPLAY_TAG(Data_MovementSpeed, "Data.Stat.MovementSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_DamageNumber_PredictedOnly, "Data.UI.DamageNumberPredictedOnly");
	UE_DEFINE_GAMEPLAY_TAG(Data_Consumable_HealAmount, "Data.Consumable.HealAmount");
	UE_DEFINE_GAMEPLAY_TAG(Data_Consumable_Duration,   "Data.Consumable.Duration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Consumable_StaminaCostMultiplier, "Data.Consumable.StaminaCostMultiplier");

	// ─── Consumables ──────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable_BloodRoot, "Item.Consumable.BloodRoot");
	UE_DEFINE_GAMEPLAY_TAG(Item_Consumable_GulSerum,  "Item.Consumable.GulSerum");

	// ─── Hitbox Parts ──────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Body_WeakSpot,    "Body.WeakSpot");
	UE_DEFINE_GAMEPLAY_TAG(Body_ArmoredLimb, "Body.ArmoredLimb");

	// ─── Kill Conditions ───────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Kill_Melee,              "Kill.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Kill_WeakSpot,           "Kill.WeakSpot");
	UE_DEFINE_GAMEPLAY_TAG(Kill_MultiTarget_Count3, "Kill.MultiTarget.Count3");

	// ─── Checkpoints ───────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Checkpoint_MidBoss,  "Checkpoint.MidBoss");
	UE_DEFINE_GAMEPLAY_TAG(Checkpoint_MainBoss, "Checkpoint.MainBoss");

	// ─── Weapon Slot ───────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Primary,   "Weapon.Primary");
	UE_DEFINE_GAMEPLAY_TAG(Weapon_Secondary, "Weapon.Secondary");

	// ─── Shoot Animation ─────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Animation_Fire_RustyLeverAction,		"Animation.Fire.RustyLeverAction");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Fire_Sporebloom,			"Animation.Fire.Sporebloom");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Fire_ChicagoTypewriter,	"Animation.Fire.ChicagoTypewriter");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Fire_SecondaryCommon,		"Animation.Fire.SecondaryCommon");
	
	// ─── Reload Animation ─────────────────────────────────────────────────────
	
	UE_DEFINE_GAMEPLAY_TAG(Animation_Reload_ChicagoTypewriter, "Animation.Reload.ChicagoTypewriter");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Reload_RepeaterPistol,    "Animation.Reload.RepeaterPistol");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Reload_Sporebloom,        "Animation.Reload.Sporebloom");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Reload_Meridian,          "Animation.Reload.Meridian");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Reload_RustyLeverAction,  "Animation.Reload.RustyLeverAction");
	UE_DEFINE_GAMEPLAY_TAG(Animation_Reload_DoubleBarrel,      "Animation.Reload.DoubleBarrel");

	// ─── Lobby ─────────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(LobbyTag_InfiniteAmmo, "LobbyTag.InfiniteAmmo");

	// ─── Gameplay Cues ─────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Character_Hit, "GameplayCue.Character.Hit");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Fire,   "GameplayCue.Weapon.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Reload, "GameplayCue.Weapon.Reload");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_MeridianGrenade_Explosion, "GameplayCue.Weapon.MeridianGrenade.Explosion");
	
	// ─── Ravager Abilities ──────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Bite_Single, "Ability.Ravager.Bite.Single");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_FromSingle_SwipeL, "Ability.Ravager.FromSingle.SwipeL");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_FromSingle_SwipeR, "Ability.Ravager.FromSingle.SwipeR");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_FromSingle_Double, "Ability.Ravager.FromSingle.Double");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Swipe_L, "Ability.Ravager.Swipe.L");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Swipe_LFromR, "Ability.Ravager.Swipe.LFromR");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Swipe_R, "Ability.Ravager.Swipe.R");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Swipe_RFromL, "Ability.Ravager.Swipe.RFromL");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_FlashKick, "Ability.Ravager.FlashKick");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Turn, "Ability.Ravager.Turn");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Evade_L, "Ability.Ravager.Evade.L");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Evade_R, "Ability.Ravager.Evade.R");
	
	// ─── Wraith Abilities ──────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_Wraith_FireTwinArrows, "Ability.Wraith.FireTwinArrows");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Wraith_FireTwinArrows, "Cooldown.Wraith.FireTwinArrows");
	UE_DEFINE_GAMEPLAY_TAG(Event_Wraith_FireTwinArrows_Shot, "Event.Wraith.FireTwinArrows.Shot");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Wraith_Teleport,"Ability.Wraith.Teleport");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Wraith_Teleport, "Cooldown.Wraith.Teleport");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Wraith_BowShove,"Ability.Wraith.BowShove");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Wraith_BowShove, "Cooldown.Wraith.BowShove");
	
	// ─── Wraith Gameplay Cues ──────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Fire, "GameplayCue.Wraith.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Teleport_Start, "GameplayCue.Wraith.Teleport.Start");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Teleport_End, "GameplayCue.Wraith.Teleport.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_Wraith_Teleport_Vanish, "Event.Wraith.Teleport.Vanish");
	UE_DEFINE_GAMEPLAY_TAG(Event_Wraith_Teleport_Appear , "Event.Wraith.Teleport.Appear");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_BowShove, "GameplayCue.Wraith.BowShove");
	
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OnCollision, "Event.Enemy.Attack.OnCollision");
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OffCollision, "Event.Enemy.Attack.OffCollision");

	// ─── Montage Events ───────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ConsumableApply, "Event.Montage.ConsumableApply");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_RelicApply, "Event.Montage.RelicApply");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_FireWeaponStart, "Event.Montage.Fire.WeaponStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ReloadWeaponStart, "Event.Montage.Reload.WeaponStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ReloadAmmoCommit, "Event.Montage.Reload.AmmoCommit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_WeaponSwapCommit, "Event.Montage.WeaponSwap.Commit");
	
}
