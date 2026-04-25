# Project Blackout — AI/Boss 에픽 클래스 다이어그램 인덱스

> 각 레이어별 상세 다이어그램은 아래 파일을 참조하세요.
> TDD v5 §2, §6, §6.1, §8, §11 및 GDD §5, §6, §6.0 기반.

| # | 파일 | 내용 |
|---|------|------|
| 01 | [01_Enemy_Boss_Hierarchy.md](01_Enemy_Boss_Hierarchy.md) | ABlackoutEnemyCharacter 확장 + ABlackoutBossCharacter + 구체 미니언/보스 |
| 02 | [02_AI_Controllers.md](02_AI_Controllers.md) | ABlackoutAIController / 미니언(순수 StateTree) · 보스(StateTree + 페이즈별 하위 BT) 컨트롤러 |
| 03 | [03_StateTree_SubBT_Assets.md](03_StateTree_SubBT_Assets.md) | StateTree 자산(미니언/보스 페이즈) + 어그로 Evaluator + 보스 페이즈별 하위 BehaviorTree |
| 04 | [04_Boss_Abilities.md](04_Boss_Abilities.md) | 슈루드 / 타락한 약탈자 페이즈별 GA 세트 |
| 05 | [05_Destructible_Pillar.md](05_Destructible_Pillar.md) | ABlackoutDestructiblePillar — Chaos Geometry Collection 파괴 |
| 06 | [06_Dependency_Overview.md](06_Dependency_Overview.md) | AI/Boss 레이어 전체 의존 관계 및 구현 순서 |

## 에픽 범위 요약

- **미니언**: `Root Hollow`(일반, 박치기 Stagger 돌진), `Root Wraith`(엘리트, 2연발 투사체 + 점멸 + 근접 밀치기)
- **중간 보스**: `Shrewd` — **2-Phase Cycling**(원거리 발판 / 근접 지면), LoS 차단 시 즉각 텔레포트 후 근접 콤보, 공통 어그로(§6.0) 적용. **씨앗 기믹은 기획 보류(제거 가능)** — 스켈레톤만 유지.
- **메인 보스**: `Corrupted Ravager` — 3-Phase (기동성 → 붉은안개/혼합소환 → 광폭화/Gorenado), 기둥 파괴는 돌진 계열 GA의 부차 효과
- **공통 모듈**: `FBSTEval_AggroTarget` StateTree Evaluator (Shrewd/Ravager **공통** 어그로 평가), `UBOBossData` (페이즈·어그로 튜닝)
- **환경 오브젝트**: `ABlackoutDestructiblePillar` (Phase A/B 진행 중 돌진 히트로 순차 파괴되어 Phase C 회피 난이도 상승)

## AI 아키텍처 원칙 ⭐

- **미니언**: **순수 StateTree**. 이동/공격/점멸 등 모든 행동을 StateTreeTask로 구현.
- **보스**: **StateTree가 페이즈 시스템을 관리**하고, **각 페이즈 상태가 하위 BehaviorTree**를 실행하여 해당 페이즈의 전투 패턴을 돌림. 페이즈 간 공통 전이(사망/이벤트 인터럽트)는 StateTree의 Transition으로 일원화.
- **어그로(타겟 선정)**: 별도 컴포넌트 없음. `FBSTEval_AggroTarget` **StateTree Evaluator**가 누적 피해 추적·감쇠·3순위 평가를 모두 담당하여 ST 컨텍스트 및 Blackboard에 타겟을 퍼블리시. (BT 기반 AI에서의 BTService 패턴의 StateTree 대응)
- **플러그인 의존**: `StateTree` + `GameplayStateTree`(GAS·GameplayTag 연동 Task 제공) — `.uproject`에 이미 활성화됨.

## 선행 의존

- Foundation 에픽: `ABlackoutEnemyCharacter`, `IBlackoutPoolableInterface`, `UBlackoutPoolSubsystem`, `UBlackoutBaseAttributeSet`
- Combat 에픽: `GE_Damage`, `UBlackoutHitboxComponent`, `ExecCalc_DamageCalc` (약점 배율)
