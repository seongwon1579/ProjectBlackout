# AI/Boss — 06. 의존 관계 및 구현 순서

> 전체 AI/Boss 레이어가 어떻게 맞물리는지, 그리고 어떤 순서로 구현하면 블로킹 없이 진행되는지 요약.
> **AI 프레임워크: 미니언 = 순수 StateTree, 보스 = StateTree(페이즈 + 어그로 Evaluator) + 하위 BT(페이즈별 패턴).**

## 의존 그래프

```mermaid
classDiagram
    direction LR

    class FoundationLayer {
        <<선행>>
        ABlackoutEnemyCharacter
        IBlackoutPoolableInterface
        UBlackoutPoolSubsystem
        UBlackoutBaseAttributeSet
        UBOMinionData / UBOBossData
    }

    class CombatLayer {
        <<선행>>
        GE_Damage
        ExecCalc_DamageCalc
        UBlackoutHitboxComponent
    }

    class AIControllerLayer {
        ABlackoutAIController (StateTreeAIComp)
        ABlackoutMinionAIController
        ABlackoutBossAIController (+BTComp,BBComp)
    }

    class StateTreeAssets {
        ST_RootHollow / ST_RootWraith
        ST_Shrewd_Phases / ST_Ravager_Phases
        FBSTTask_* / FBSTCond_*
        FBSTEval_HealthRatio
        FBSTEval_ShrewdAggroTarget / WraithAggroTarget
        UBlackoutAggroEvaluator ★ 보스 어그로 평가
    }

    class SubBTAssets {
        BT_Shrewd_Platform / Ground
        BT_Ravager_PhaseA/B/C
        UBTTask_ActivateBossAbility
        UBTService_LineOfSightCheck
    }

    class CharacterLayer {
        ABORootHollow / ABORootWraith
        ABlackoutBossCharacter
        ABOShrewdBoss / ABORavagerBoss
    }

    class AbilityLayer {
        UBlackoutGA_Shrewd_*
        UBlackoutGA_Ravager_*
    }

    class PillarLayer {
        ABOBreakablePillarActor
        GameState::DestroyedPillarIds
    }

    FoundationLayer --> CharacterLayer
    FoundationLayer --> AIControllerLayer
    CombatLayer --> AbilityLayer
    CombatLayer --> PillarLayer
    AIControllerLayer --> StateTreeAssets : StateTreeAIComp.StartLogic
    StateTreeAssets --> SubBTAssets : FBSTTask_RunSubBehaviorTree
    AIControllerLayer --> SubBTAssets : BehaviorTreeComp (보스 전용)
    CharacterLayer --> StateTreeAssets : ST 에셋 참조
    StateTreeAssets --> AIControllerLayer : Aggro Evaluator → Blackboard target
    AbilityLayer --> CharacterLayer : GrantedAbilities
    AbilityLayer --> PillarLayer : 돌진 계열 GA 히트 부차효과<br/>(LungeAttackCombo, ChargedShockwave)
    SubBTAssets --> AbilityLayer : UBTTask_ActivateBossAbility
    StateTreeAssets --> AbilityLayer : FBSTTask_ActivateAbility (미니언 공격)
    CombatLayer --> StateTreeAssets : GE_Damage → ASC Delegate → AggroEvaluator
```

## 권장 구현 순서

| 단계 | 작업 | 산출물 | 검증 |
|---|---|---|---|
| 1 | `ABlackoutAIController` 베이스 + `UStateTreeAIComponent` 부착 | 02 | Pawn Possess 시 StateTree 시작 로그 |
| 2 | StateTree 공용 기반 Task/Cond/Eval(`FBSTTask_ActivateAbility`, `FBSTEval_HealthRatio` 등) | 03 | 빈 ST에서 Ability 발동 테스트 |
| 3 | `ABORootHollow` + `ST_RootHollow` + `FBSTTask_Charge` | 01, 03 | 더미 플레이어 추격·돌진 Stagger |
| 4 | `ABORootWraith` + `ST_RootWraith` + `FBSTTask_Teleport` / `FBSTTask_FireTwinArrows` / **`FBSTTask_BowShove`** | 01, 03 | 2연발 후 시야 밖 점멸, 근접 감지 시 활대 밀치기 |
| 5 | `ABlackoutBossCharacter` 추상 베이스 + 페이즈 enum/이벤트 | 01 | HP 비율 컷라인 돌파 시 `OnPhaseChanged` 호출 |
| 6 | `ABlackoutBossAIController` + `BTComp`/`BBComp` + `WriteTargetToBlackboard` | 02 | BB 키 수동 기록 테스트 |
| 7 | **`UBlackoutAggroEvaluator` + StateTree Aggro Evaluator** (누적 피해 추적·감쇠·타겟 선정·BB 기록) | 02, 03 | 피해 집중 플레이어로 타겟 전환·쿨다운·감쇠 동작 확인 |
| 8 | `FBSTTask_RunSubBehaviorTree` + `FBSTCond_HealthBelow` 구현 | 03 | 빈 ST(PhaseA→B)에서 하위 BT 기동/정지 전이 |
| 9 | `ABOShrewdBoss` + `ST_Shrewd_Phases` + Shrewd GA(`FireExplosiveArrow`, `FireStraightArrow`, `TeleportToPoint`, `TeleportByEQS`) | 01, 03, 04 | 원거리 화살, EQS/지점 텔레포트 패턴 |
| 10 | `ABORavagerBoss` + `ST_Ravager_Phases` + Phase A/B/C BT + Ravager GA 세트(`BasicAttack`, `ChaseAttack`, `Charge`, `Shockwave`, `EnergyBurst`, `Gorenado`, `SummonMinion`) | 01, 03, 04 | Phase별 패턴 순환 |
| 11 | `UBlackoutGA_Ravager_SummonMinion` + 풀 서브시스템 스폰 경로 | 04 | 일반+엘리트 혼합 스폰 |
| 12 | `ABOBreakablePillarActor` + Ravager GA 피해 스펙 연결 | 05 | Phase A/B 전투 중 기둥 순차 파괴 |
| 14 | `GameState::DestroyedPillarIds` 동기화 + Phase C 난이도 반영 | 05 | Late-join 시 잔해 재현 |

## 핵심 교차 검증 포인트

- **StateTree ↔ 하위 BT 경계**: 페이즈 전이는 **오직 StateTree만** 수행하도록 강제. 하위 BT에서 페이즈 변경을 시도하는 노드가 없어야 함(그렇지 않으면 두 레이어가 상태를 경쟁).
- **어그로 Evaluator ↔ BB**: `UBlackoutAggroEvaluator` / StateTree Aggro Evaluator가 현재 타겟을 계산하고, 컨트롤러 Blackboard에 `BB_CurrentTarget`을 기록합니다. `ABOShrewdBoss`의 `UBlackoutAggroComponent`는 현재 코드에 남아 있는 보조/디버그 경로입니다.
- **StateTree 외부 데이터 수명**: `FStateTreeExternalDataHandle`로 주입되는 ASC·Controller·BossData는 Pawn/Controller 수명 동안 유효해야 함 — 풀 반환 시점(보스는 풀 미대상이지만 안전 가드 목적)에 StateTree를 먼저 Stop.
- **서버 권한**: Evaluator의 `HandleDamageReceived`는 서버 ASC 델리게이트에서만 호출됨. 클라이언트 ST 인스턴스에서는 `TreeStart` 시 NM_Client 가드로 조기 리턴.
- **풀링 ↔ 스폰**: `UBlackoutGA_Ravager_SummonMinion`은 `UBlackoutPoolSubsystem` 경유로 미니언을 꺼냅니다. 직접 `SpawnActor`는 특수 연출/비풀링 대상에만 제한합니다.
- **기둥 파괴 경로**: PillarCharge 단일 GA에 의존하지 않고, Ravager 계열 GA의 피해 스펙이 `ABOBreakablePillarActor`에 들어오면 서버에서 출처를 검증한 뒤 `BreakPillar()`를 호출합니다. 이 제한으로 플레이어 무기가 기둥을 파괴하지 못하도록 서버에서 강제합니다.
- **공통 어그로 튜닝**: Shrewd, Ravager의 스위치 쿨다운/피해 임계/감쇠율은 `UBOBossData`의 어그로 파라미터를 기준으로 주입합니다.
- **데이터 기반**: 어그로 파라미터, 페이즈 컷라인, 패턴 데미지는 모두 `UBOBossData`에서 주입. StateTree 파라미터 바인딩으로 노출하여 BP·에디터에서 수정 가능.
- **디버깅 분리**: 페이즈 전이 문제는 StateTree Debugger, 패턴 선택 문제는 BT Visual Logger로 각각 격리 추적.
