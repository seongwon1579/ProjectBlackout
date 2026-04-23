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

	// ─── Lobby ─────────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(LobbyTag_InfiniteAmmo, "LobbyTag.InfiniteAmmo");

	// ─── Gameplay Cues ─────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Character_Hit, "GameplayCue.Character.Hit");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Fire,   "GameplayCue.Weapon.Fire");
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Reload, "GameplayCue.Weapon.Reload");
}
