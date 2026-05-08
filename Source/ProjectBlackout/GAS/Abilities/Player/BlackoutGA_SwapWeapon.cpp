#include "GAS/Abilities/Player/BlackoutGA_SwapWeapon.h"

#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Core/BlackoutLog.h"
#include "GameplayTags/BlackoutGameplayTags.h"

UBlackoutGA_SwapWeapon::UBlackoutGA_SwapWeapon()
{
	InputID = EBlackoutAbilityInputID::SwapWeapon;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FGameplayTagContainer AssetTags;
	AssetTags.AddTag(BlackoutGameplayTags::Ability_Player_SwapWeapon);
	SetAssetTags(AssetTags);
	CancelAbilitiesWithTag.AddTag(BlackoutGameplayTags::Ability_Player_Reload);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Downed);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Locked);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Attacking);
}

void UBlackoutGA_SwapWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_SwapWeapon activate requested");

	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	if (!PlayerCharacter || !CombatComponent)
	{
		BO_LOG_GAS(Warning, "GA_SwapWeapon failed: PlayerCharacter 또는 CombatComponent가 유효하지 않음");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_SwapWeapon failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	CombatComponent->SwapWeapon();
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
