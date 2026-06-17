# Project Blackout — AI/Boss 에픽 클래스 다이어그램 인덱스

> 각 레이어별 상세 다이어그램은 아래 파일을 참조하세요.
> TDD v5 §2, §6, §6.1, §8, §11 및 GDD §5, §6, §6.0 기반.

| # | 파일 | 내용 |
|---|------|------|
| 01 | [01_Enemy_Boss_Hierarchy.md](01_Enemy_Boss_Hierarchy.md) | ABlackoutEnemyCharacter / ABlackoutMinionCharacter + ABlackoutBossCharacter + 구체 미니언/보스 |
| 02 | [02_AI_Controllers.md](02_AI_Controllers.md) | ABlackoutAIController / 미니언·Shrewd(순수 StateTree) · Ravager(순수 BehaviorTree + C++ 페이즈 모듈) 컨트롤러 |
| 03 | [03_StateTree_SubBT_Assets.md](03_StateTree_SubBT_Assets.md) | StateTree 자산(미니언/Shrewd) + Ravager BehaviorTree·페이즈 모듈(PhaseEvaluator/BTRunner) + 어그로 Evaluator |
| 04 | [04_Boss_Abilities.md](04_Boss_Abilities.md) | 슈루드 / 타락한 약탈자 GA 세트 |
| 05 | [05_Destructible_Pillar.md](05_Destructible_Pillar.md) | ABOBreakablePillarActor — ChildActor 파편 전환 기반 기둥 파괴 |
| 06 | [06_Dependency_Overview.md](06_Dependency_Overview.md) | AI/Boss 레이어 전체 의존 관계 및 구현 순서 |

## 에픽 범위 요약

- **미니언**: `Root Hollow`(일반, 박치기 Stagger 돌진), `Root Wraith`(엘리트, 2연발 투사체 + 점멸 + 근접 밀치기)
- **중간 보스**: `Shrewd` — **순수 StateTree** 단일 비행 페이즈, 원거리 화살 + LoS 차단 시 즉각 텔레포트, 공통 어그로(§6.0) 적용.
- **메인 보스**: `Corrupted Ravager` — **순수 BehaviorTree** 3-Phase (기동성 → 강화 패턴/혼합소환 → 광폭화/Gorenado), C++ 페이즈 모듈로 관리, 기둥 파괴는 돌진 계열 GA의 부차 효과
- **공통 모듈**: `UBlackoutAggroEvaluator`(가중치 점수 어그로), `UBlackoutPhaseEvaluator`/`UBlackoutBossBTRunner`(Ravager 페이즈), `UBOBossData`(페이즈 컷라인·부위 배율)
- **환경 오브젝트**: `ABOBreakablePillarActor` (Phase 진행 중 Ravager 계열 히트로 순차 파괴되어 Phase3 회피 난이도 상승)

## AI 아키텍처 원칙 ⭐ (v6)

- **미니언**: **순수 StateTree**. 이동/공격/점멸 등 모든 행동을 StateTreeTask로 구현.
- **중간 보스(Shrewd)**: **순수 StateTree**. 페이즈 관리 모듈·하위 BT 없이 StateTree만으로 패턴을 순환.
- **메인 보스(Ravager)**: **순수 BehaviorTree**. 페이즈 관리는 C++ `UBlackoutPhaseEvaluator`(`Ability.PhaseLock` 태그 게이팅), 페이즈별 BehaviorTree 교체는 `UBlackoutBossBTRunner`. 페이즈 결정은 보스 캐릭터(`ABORavagerBoss::OnDamageReceived → DetermineTargetPhase`). 패턴 선택/실행은 BT 노드(`BTT_PickNextPattern`/`SelectAbility`/`ActivateAbility`).
- **어그로(타겟 선정)**: `ABlackoutBossAIController`가 소유하는 `UBlackoutAggroEvaluator`가 **가중치 점수제**(DPS 3초 윈도우·거리·저체력)로 타겟을 평가하고 `OnAggroTargetChanged`로 전파합니다. Shrewd StateTree는 Pawn의 `UBlackoutAggroComponent`가 제공하는 현재 타겟을 컨텍스트로 사용합니다.
- **플러그인 의존**: `StateTree` + `GameplayStateTree` — `.uproject`에 이미 활성화됨(미니언·Shrewd용).

## 선행 의존

- Foundation 에픽: `ABlackoutEnemyCharacter`, `ABlackoutMinionCharacter`, `IBlackoutPoolableInterface`, `UBlackoutPoolSubsystem`, `UBlackoutBaseAttributeSet`
- Combat 에픽: `GE_Damage`, `UBlackoutHitboxComponent`, `ExecCalc_DamageCalc` (약점 배율)
