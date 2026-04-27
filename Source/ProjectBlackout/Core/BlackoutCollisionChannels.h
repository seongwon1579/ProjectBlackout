#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

namespace BlackoutCollisionChannels
{
	/** 무기 조준 및 사격 판정에 사용하는 전용 Trace Channel. DefaultEngine.ini의 WeaponTrace와 일치해야 합니다. */
	inline constexpr ECollisionChannel WeaponTrace = ECC_GameTraceChannel1;
}
