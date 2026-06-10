#pragma once

struct FGameplayAbilityActorInfo;
class ABlackoutPlayerState;

namespace BlackoutAbilityUtils
{
	const ABlackoutPlayerState* ResolveOwningBlackoutPlayerState(const FGameplayAbilityActorInfo* ActorInfo);
}