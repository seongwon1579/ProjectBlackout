#include "BlackoutGameplayTags.h"

namespace BlackoutGameplayTags
{
	// ─── Character State ───────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Downed,        "State.Downed");
	UE_DEFINE_GAMEPLAY_TAG(State_Reviving,      "State.Reviving");
	UE_DEFINE_GAMEPLAY_TAG(State_BeingRevived,  "State.BeingRevived");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead,          "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Invulnerable,  "State.Invulnerable");
	UE_DEFINE_GAMEPLAY_TAG(State_MovementLocked,"State.MovementLocked");
	UE_DEFINE_GAMEPLAY_TAG(State_Locked,        "State.Locked");
	UE_DEFINE_GAMEPLAY_TAG(State_Stunned,       "State.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_StunBroken,    "State.StunBroken");
	UE_DEFINE_GAMEPLAY_TAG(State_Aiming,        "State.Aiming");
	UE_DEFINE_GAMEPLAY_TAG(State_Sprinting,     "State.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking,     "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Reloading,     "State.Reloading");
	UE_DEFINE_GAMEPLAY_TAG(State_UseConsumable, "State.UseConsumable");
	UE_DEFINE_GAMEPLAY_TAG(State_UseRelic,      "State.UseRelic");
	UE_DEFINE_GAMEPLAY_TAG(State_Pulled,		"State.Pulled");
	
	// ─── Character Abilities ───────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Dodge,			"Ability.Player.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Player_Melee,			"Ability.Player.Melee");
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
	UE_DEFINE_GAMEPLAY_TAG(Data_Stun,          "Data.Stun");
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

	// ─── Impact Surface ───────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Surface_Default, "Surface.Default");
	UE_DEFINE_GAMEPLAY_TAG(Surface_Flesh,   "Surface.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(Surface_Metal,   "Surface.Metal");
	UE_DEFINE_GAMEPLAY_TAG(Surface_Stone,   "Surface.Stone");

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
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Character_Revive, "GameplayCue.Character.Revive");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Consumable_Use, "GameplayCue.Consumable.Use");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Consumable_BloodRoot_Use, "GameplayCue.Consumable.BloodRoot.Use");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Consumable_GulSerum_Use, "GameplayCue.Consumable.GulSerum.Use");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Relic_Use, "GameplayCue.Relic.Use");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Default_Fire, "GameplayCue.Weapon.Default.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Default_Trail, "GameplayCue.Weapon.Default.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Default_Impact_Default, "GameplayCue.Weapon.Default.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Default_Impact_Flesh, "GameplayCue.Weapon.Default.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Default_Impact_Metal, "GameplayCue.Weapon.Default.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Default_Impact_Stone, "GameplayCue.Weapon.Default.Impact.Stone");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Fire, "GameplayCue.Weapon.ChicagoTypewriter.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Trail, "GameplayCue.Weapon.ChicagoTypewriter.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Impact_Default, "GameplayCue.Weapon.ChicagoTypewriter.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Impact_Flesh, "GameplayCue.Weapon.ChicagoTypewriter.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Impact_Metal, "GameplayCue.Weapon.ChicagoTypewriter.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Impact_Stone, "GameplayCue.Weapon.ChicagoTypewriter.Impact.Stone");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Reload_In, "GameplayCue.Weapon.ChicagoTypewriter.Reload.In");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_ChicagoTypewriter_Reload_Out, "GameplayCue.Weapon.ChicagoTypewriter.Reload.Out");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RepeaterPistol_Fire, "GameplayCue.Weapon.RepeaterPistol.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RepeaterPistol_Trail, "GameplayCue.Weapon.RepeaterPistol.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RepeaterPistol_Impact_Default, "GameplayCue.Weapon.RepeaterPistol.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RepeaterPistol_Impact_Flesh, "GameplayCue.Weapon.RepeaterPistol.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RepeaterPistol_Impact_Metal, "GameplayCue.Weapon.RepeaterPistol.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RepeaterPistol_Impact_Stone, "GameplayCue.Weapon.RepeaterPistol.Impact.Stone");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RepeaterPistol_Reload, "GameplayCue.Weapon.RepeaterPistol.Reload");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Fire, "GameplayCue.Weapon.Sporebloom.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Trail, "GameplayCue.Weapon.Sporebloom.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Impact_Default, "GameplayCue.Weapon.Sporebloom.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Impact_Flesh, "GameplayCue.Weapon.Sporebloom.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Impact_Metal, "GameplayCue.Weapon.Sporebloom.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Impact_Stone, "GameplayCue.Weapon.Sporebloom.Impact.Stone");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Reload_Rack, "GameplayCue.Weapon.Sporebloom.Reload.Rack");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Reload_Release, "GameplayCue.Weapon.Sporebloom.Reload.Release");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Sporebloom_Reload_Reload, "GameplayCue.Weapon.Sporebloom.Reload.Reload");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Fire, "GameplayCue.Weapon.Meridian.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Trail, "GameplayCue.Weapon.Meridian.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Impact_Default, "GameplayCue.Weapon.Meridian.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Impact_Flesh, "GameplayCue.Weapon.Meridian.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Impact_Metal, "GameplayCue.Weapon.Meridian.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Impact_Stone, "GameplayCue.Weapon.Meridian.Impact.Stone");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Explosion, "GameplayCue.Weapon.Meridian.Explosion");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Reload_In, "GameplayCue.Weapon.Meridian.Reload.In");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Meridian_Reload_Out, "GameplayCue.Weapon.Meridian.Reload.Out");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Fire, "GameplayCue.Weapon.RustyLeverAction.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Trail, "GameplayCue.Weapon.RustyLeverAction.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Impact_Default, "GameplayCue.Weapon.RustyLeverAction.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Impact_Flesh, "GameplayCue.Weapon.RustyLeverAction.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Impact_Metal, "GameplayCue.Weapon.RustyLeverAction.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Impact_Stone, "GameplayCue.Weapon.RustyLeverAction.Impact.Stone");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Bolt_Push, "GameplayCue.Weapon.RustyLeverAction.Bolt.Push");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Bolt_Pull, "GameplayCue.Weapon.RustyLeverAction.Bolt.Pull");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Reload_Mag_In, "GameplayCue.Weapon.RustyLeverAction.Reload.Mag.In");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Reload_Mag_Out, "GameplayCue.Weapon.RustyLeverAction.Reload.Mag.Out");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Reload_Bolt_Push, "GameplayCue.Weapon.RustyLeverAction.Reload.Bolt.Push");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_RustyLeverAction_Reload_Bolt_Pull, "GameplayCue.Weapon.RustyLeverAction.Reload.Bolt.Pull");

	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Fire, "GameplayCue.Weapon.DoubleBarrel.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Trail, "GameplayCue.Weapon.DoubleBarrel.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Impact_Default, "GameplayCue.Weapon.DoubleBarrel.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Impact_Flesh, "GameplayCue.Weapon.DoubleBarrel.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Impact_Metal, "GameplayCue.Weapon.DoubleBarrel.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Impact_Stone, "GameplayCue.Weapon.DoubleBarrel.Impact.Stone");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Reload_Open, "GameplayCue.Weapon.DoubleBarrel.Reload.Open");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Reload_Insert, "GameplayCue.Weapon.DoubleBarrel.Reload.Insert");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_DoubleBarrel_Reload_Close, "GameplayCue.Weapon.DoubleBarrel.Reload.Close");
	
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_SteelSword_Swing, "GameplayCue.Weapon.SteelSword.Swing");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_SteelSword_Impact_Default, "GameplayCue.Weapon.SteelSword.Impact.Default");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_SteelSword_Impact_Flesh, "GameplayCue.Weapon.SteelSword.Impact.Flesh");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_SteelSword_Impact_Metal, "GameplayCue.Weapon.SteelSword.Impact.Metal");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_SteelSword_Impact_Stone, "GameplayCue.Weapon.SteelSword.Impact.Stone");
	
	// ─── Ravager Abilities ──────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_PhaseLock, "Ability.PhaseLock");
	UE_DEFINE_GAMEPLAY_TAG(Ability_TargetChange, "Ability.TargetChange");
	
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
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Turn_L_90, "Ability.Ravager.Turn.L.90");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Turn_L_135, "Ability.Ravager.Turn.L.135");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Turn_L_180, "Ability.Ravager.Turn.L.180");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Turn_R_90, "Ability.Ravager.Turn.R.90");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Turn_R_135, "Ability.Ravager.Turn.R.135");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Turn_R_180, "Ability.Ravager.Turn.R.180");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Evade_L, "Ability.Ravager.Evade.L");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Evade_R, "Ability.Ravager.Evade.R");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Shockwave, "Ability.Ravager.Shockwave");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_SpawnMinion, "Ability.Ravager.SpawnMinion");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Gorenado, "Ability.Ravager.Gorenado");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_EnergyBurst, "Ability.Ravager.EnergyBurst");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Charge, "Ability.Ravager.Charge");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ravager_Howl, "Ability.Ravager.Howl");
	
	// ─── Ravager Gameplay Cues ──────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_Gorenado, "GameplayCue.Ravager.Gorenado");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_EnergyBurst, "GameplayCue.Ravager.EnergyBurst");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_Shockwave_Launch, "GameplayCue.Ravager.Shockwave.Launch");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_Swipe_L, "GameplayCue.Ravager.Swipe.L");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_Swipe_R, "GameplayCue.Ravager.Swipe.R");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_BiteSingle, "GameplayCue.Ravager.BiteSingle");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_Evade, "GameplayCue.Ravager.Evade");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Ravager_FlashKick, "GameplayCue.Ravager.FlashKick");
	
	// ─── Hollow Abilities ──────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_Hollow_Spawn, "Ability.Hollow.Spawn");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Hollow_PreRoll, "Ability.Hollow.PreRoll");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Hollow_Attack, "Ability.Hollow.Attack");
	
	// ─── Shrewd Abilities ──────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_Shrewd_Teleport_ToPoint, "Ability.Shrewd.Teleport.ToPoint");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Shrewd_Teleport_ByEQS, "Ability.Shrewd.Teleport.ByEQS");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Shrewd_Teleport, "Cooldown.Shrewd.Teleport");
	
	UE_DEFINE_GAMEPLAY_TAG(Ability_Shrewd_Fire_Arrow_Straight, "Ability.Shrewd.Fire.Arrow.Straight");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Shrewd_Fire_Arrow_Explosive, "Ability.Shrewd.Fire.Arrow.Explosive");
	
	// ─── Shrewd Gameplay Cues ──────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Shrewd_Projectile_Trail, "GameplayCue.Shrewd.Projectile.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Shrewd_Projectile_Explosion, "GameplayCue.Shrewd.Projectile.Explosion");
	
	// ─── Wraith Abilities ──────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Ability_Wraith_FireTwinArrows, "Ability.Wraith.FireTwinArrows");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Wraith_FireTwinArrows, "Cooldown.Wraith.FireTwinArrows");
	UE_DEFINE_GAMEPLAY_TAG(Event_Wraith_FireTwinArrows_Shot, "Event.Wraith.FireTwinArrows.Shot");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Wraith_Teleport,"Ability.Wraith.Teleport");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Wraith_Teleport, "Cooldown.Wraith.Teleport");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Wraith_BowShove,"Ability.Wraith.BowShove");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Wraith_BowShove, "Cooldown.Wraith.BowShove");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Wraith_Teleport_Vanish, "Event.Wraith.Teleport.Vanish");
	UE_DEFINE_GAMEPLAY_TAG(Event_Wraith_Teleport_Appear , "Event.Wraith.Teleport.Appear");
	
	// ─── Wraith Gameplay Cues ──────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Fire, "GameplayCue.Wraith.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Teleport_Start, "GameplayCue.Wraith.Teleport.Start");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Teleport_End, "GameplayCue.Wraith.Teleport.End");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_BowShove, "GameplayCue.Wraith.BowShove");
	
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Projectile_Trail, "GameplayCue.Wraith.Projectile.Trail");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Wraith_Projectile_Explosion, "GameplayCue.Wraith.Projectile.Explosion");
	
	// ─── Attack Events ────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OnCollision, "Event.Enemy.Attack.OnCollision");
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OffCollision, "Event.Enemy.Attack.OffCollision");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OnGorenado, "Event.Enemy.Attack.OnGorenado");
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OffGorenado, "Event.Enemy.Attack.OffGorenado");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_Spawn_Projectile, "Event.Enemy.Attack.Spawn.Projectile");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_Charge_LoopStart, "Event.Enemy.Attack.Charge.LoopStart");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OnEnergyBurst, "Event.Enemy.Attack.OnEnergyBurst");
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_OffEnergyBurst, "Event.Enemy.Attack.OffEnergyBurst");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_KickFlash_Start, "Event.Enemy.Attack.KickFlash.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Attack_KickFlash_End, "Event.Enemy.Attack.KickFlash.End");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Enemy_Shrewd_Attack_FireArrow, "Event.Enemy.Shrewd.Attack.FireArrow");
	
	// ─── Montage Events ───────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ConsumableApply, "Event.Montage.ConsumableApply");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_RelicApply, "Event.Montage.RelicApply");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_FireWeaponStart, "Event.Montage.Fire.WeaponStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ReloadWeaponStart, "Event.Montage.Reload.WeaponStart");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_ReloadAmmoCommit, "Event.Montage.Reload.AmmoCommit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_WeaponSwapCommit, "Event.Montage.WeaponSwap.Commit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_AbilityCancelable, "Event.Montage.AbilityCancelable");

	// ─── Shelter ──────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_InShelter,        "State.InShelter");
	UE_DEFINE_GAMEPLAY_TAG(Effect_ShelterScoped,   "Effect.ShelterScoped");
}
