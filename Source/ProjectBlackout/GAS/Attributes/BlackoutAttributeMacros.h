// ─── 구현 내역 ───────────────────────
//  - 김민영: 어트리뷰트 접근자(Getter/Setter/Initter)를 일괄 생성하는 ATTRIBUTE_ACCESSORS 매크로
// ──────────────────────────────────────

#pragma once

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)           \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)               \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
