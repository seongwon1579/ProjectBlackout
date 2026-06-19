// ─── 구현 내역 ───────────────────────
//  - 조성원: ActorInfo에서 소유 ABlackoutPlayerState를 해석하는 공용 유틸 함수(ResolveOwningBlackoutPlayerState)
// ──────────────────────────────────────

#pragma once

struct FGameplayAbilityActorInfo;
class ABlackoutPlayerState;

namespace BlackoutAbilityUtils
{
	const ABlackoutPlayerState* ResolveOwningBlackoutPlayerState(const FGameplayAbilityActorInfo* ActorInfo);
}