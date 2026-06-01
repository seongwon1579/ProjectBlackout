#include "GE_InShelter.h"

UGE_InShelter::UGE_InShelter()
{
	// 라이프사이클만 C++에서 잠금. GrantedTags / Modifier / GameplayCue 같은 디자이너 영역은
	// BP child (BP_GE_InShelter) 의 Components 배열에서 Grant Tags to Target Actor 로 처리.
	DurationPolicy = EGameplayEffectDurationType::Infinite;
}
