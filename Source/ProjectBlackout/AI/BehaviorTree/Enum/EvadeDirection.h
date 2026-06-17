// ─── 구현 내역 ───────────────────────
//  - 조성원: 회피 방향 enum 정의 (None/Left/Right/Forward/Backward)
// ──────────────────────────────────────

#pragma once

UENUM(BlueprintType)
enum class EEvadeDirection : uint8
{
	None,
	Left,
	Right,
	Forward,
	Backward,
};