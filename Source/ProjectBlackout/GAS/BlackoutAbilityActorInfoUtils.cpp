#include "BlackoutAbilityActorInfoUtils.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Framework/BlackoutPlayerState.h"

namespace BlackoutAbilityUtils
{
	const ABlackoutPlayerState* ResolveOwningBlackoutPlayerState(const FGameplayAbilityActorInfo* ActorInfo)
	{
		if (!ActorInfo)
		{
			return nullptr;
		}

		if (const ABlackoutPlayerState* PlayerState = Cast<ABlackoutPlayerState>(ActorInfo->OwnerActor.Get()))
		{
			return PlayerState;
		}

		if (const ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			return PlayerCharacter->GetPlayerState<ABlackoutPlayerState>();
		}

		return nullptr;
	}
}