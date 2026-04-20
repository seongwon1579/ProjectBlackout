# Project Blackout — AI/Boss 에픽 클래스 다이어그램 인덱스

> 각 레이어별 상세 다이어그램은 아래 파일을 참조하세요.
> TDD v5 §2, §6, §6.1, §8, §11 및 GDD §5, §6, §6.0 기반.

| # | 파일 | 내용 |
|---|------|------|
| 01 | [01_Enemy_Boss_Hierarchy.md](01_Enemy_Boss_Hierarchy.md) | ABlackoutEnemyCharacter 확장 + ABlackoutBossCharacter + 구체 미니언/보스 |
| 02 | [02_AI_Controllers.md](02_AI_Controllers.md) | ABlackoutAIController / 미니언(순수 StateTree) · 보스(StateTree + 페이즈별 하위 BT) 컨트롤러 |
| 03 | [03_Aggro_Component.md](03_Aggro_Component.md) | UBlackoutAggroComponent — 누적 피해·거리·체력 기반 타겟 선정 |
| 04 | [04_StateTree_SubBT_Assets.md](04_StateTree_SubBT_Assets.md) | StateTree 자산(미니언/보스 페이즈) + 보스 페이즈별 하위 BehaviorTree |
| 05 | [05_Boss_Abilities.md](05_Boss_Abilities.md) | 슈루드 / 타락한 약탈자 페이즈별 GA 세트 |
| 06 | [06_Destructible_Pillar.md](06_Destructible_Pillar.md) | ABlackoutDestructiblePillar — Chaos Geometry Collection 파괴 |
| 07 | [07_Dependency_Overview.md](07_Dependency_Overview.md) | AI/Boss 레이어 전체 의존 관계 및 구현 순서 |

## 에픽 범위 요약

- **미니언**: `Root Hollow`(일반, 근접 돌진), `Root Wraith`(엘리트, 2연발 투사체 + 점멸)
- **중간 보스**: `Shrewd` — 발판/지면 교대 페이즈, LoS 체크 시 즉각 점멸, 씨앗 기믹(무적 태그 부여)
- **메인 보스**: `Corrupted Ravager` — 3-Phase (기동성 → 광폭화 → 궁극기), 기둥 파괴 기믹
- **공통 모듈**: `UBlackoutAggroComponent` (보스 전용), `UBOBossData` (페이즈·어그로 튜닝)
- **환경 오브젝트**: `ABlackoutDestructiblePillar` (Phase C 난이도 가중)

## AI 아키텍처 원칙 ⭐

- **미니언**: **순수 StateTree**. 이동/공격/점멸 등 모든 행동을 StateTreeTask로 구현.
- **보스**: **StateTree가 페이즈 시스템을 관리**하고, **각 페이즈 상태가 하위 BehaviorTree**를 실행하여 해당 페이즈의 전투 패턴을 돌림. 페이즈 간 공통 전이(사망/이벤트 인터럽트)는 StateTree의 Transition으로 일원화.
- **플러그인 의존**: `StateTree` + `GameplayStateTree`(GAS·GameplayTag 연동 Task 제공) — `.uproject`에 이미 활성화됨.

## 선행 의존

- Foundation 에픽: `ABlackoutEnemyCharacter`, `IBlackoutPoolableInterface`, `UBlackoutPoolSubsystem`, `UBlackoutBaseAttributeSet`
- Combat 에픽: `GE_Damage`, `UBlackoutHitboxComponent`, `ExecCalc_DamageCalc` (약점 배율)
