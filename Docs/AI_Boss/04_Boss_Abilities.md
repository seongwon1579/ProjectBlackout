# AI/Boss — 04. 보스 어빌리티(GA) 세트

> TDD v5 §6, GDD §5·§6 참조. 현재 C++ 구현 기준의 보스 GA 클래스명과 계층을 정리합니다.

```mermaid
classDiagram
    direction TB

    UBlackoutGameplayAbility <|-- UBlackoutEnemyGameplayAbility

    UBlackoutEnemyGameplayAbility <|-- UBlackoutGA_Shrewd_Base
    UBlackoutGA_Shrewd_Base <|-- UBlackoutGA_Shrewd_FireArrowBase
    UBlackoutGA_Shrewd_FireArrowBase <|-- UBlackoutGA_Shrewd_FireExplosiveArrow
    UBlackoutGA_Shrewd_FireArrowBase <|-- UBOGA_Shrewd_FireStraightArrow
    UBlackoutGA_Shrewd_Base <|-- UBlackoutGA_Shrewd_TeleportBase
    UBlackoutGA_Shrewd_TeleportBase <|-- UBlackoutGA_Shrewd_TeleportToPoint
    UBlackoutGA_Shrewd_TeleportBase <|-- UBlackoutGA_Shrewd_TeleportByEQS

    UBlackoutEnemyGameplayAbility <|-- UBlackoutGA_Ravager_Base
    UBlackoutGA_Ravager_Base <|-- UBlackoutGA_Ravager_HitboxAttack
    UBlackoutGA_Ravager_HitboxAttack <|-- UBlackoutGA_Ravager_BasicAttack
    UBlackoutGA_Ravager_BasicAttack <|-- UBlackoutGA_Ravager_ChaseAttack
    UBlackoutGA_Ravager_HitboxAttack <|-- UBlackoutGA_Ravager_Charge
    UBlackoutGA_Ravager_Base <|-- UBlackoutGA_Ravager_Rotate
    UBlackoutGA_Ravager_Base <|-- UBlackoutGA_Ravager_Evade
    UBlackoutGA_Ravager_Base <|-- UBlackoutGA_Ravager_Shockwave
    UBlackoutGA_Ravager_Base <|-- UBlackoutGA_Ravager_EnergyBurst
    UBlackoutGA_Ravager_Base <|-- UBlackoutGA_Ravager_Gorenado
    UBlackoutGA_Ravager_Base <|-- UBlackoutGA_Ravager_SummonMinion

    class UBlackoutGA_Shrewd_FireArrowBase {
        <<Shrewd 화살 공통>>
        +ActivateAbility(...) void
    }

    class UBlackoutGA_Shrewd_TeleportBase {
        <<Shrewd 텔레포트 공통>>
        +ActivateAbility(...) void
    }

    class UBlackoutGA_Ravager_HitboxAttack {
        <<Ravager 히트박스 공격 공통>>
        +ActivateAbility(...) void
    }

    class UBlackoutGA_Ravager_SummonMinion {
        <<Root Hollow / Wraith 스폰>>
        +ActivateAbility(...) void
    }

    UBlackoutGA_Ravager_SummonMinion ..> UBlackoutPoolSubsystem : Spawn minions
    UBlackoutGA_Ravager_Base ..> ABOBreakablePillarActor : Ravager 피해 스펙 허용
```

## 페이즈 ↔ GA 매트릭스

### Shrewd (중간 보스)

| 패턴 축 | 현재 C++ GA |
|---|---|
| 폭발 화살 | `UBlackoutGA_Shrewd_FireExplosiveArrow` |
| 직선 화살 | `UBOGA_Shrewd_FireStraightArrow` |
| 지정 지점 텔레포트 | `UBlackoutGA_Shrewd_TeleportToPoint` |
| EQS 기반 텔레포트 | `UBlackoutGA_Shrewd_TeleportByEQS` |

### Ravager (메인 보스)

| 패턴 축 | 현재 C++ GA |
|---|---|
| 공통 근접 히트박스 | `UBlackoutGA_Ravager_HitboxAttack` |
| 기본/추격/돌진 공격 | `UBlackoutGA_Ravager_BasicAttack`, `UBlackoutGA_Ravager_ChaseAttack`, `UBlackoutGA_Ravager_Charge` |
| 방향 전환/회피 | `UBlackoutGA_Ravager_Rotate`, `UBlackoutGA_Ravager_Evade` |
| 광역/특수 패턴 | `UBlackoutGA_Ravager_Shockwave`, `UBlackoutGA_Ravager_EnergyBurst`, `UBlackoutGA_Ravager_Gorenado` |
| 미니언 소환 | `UBlackoutGA_Ravager_SummonMinion` |

## 구현 노트

- **Ability Tag 네이밍**: C++ 클래스명은 `UBlackoutGA_*` 계열을 사용하고, 실제 발동/데미지 데이터는 `UBOBossData::AbilityDamageMap`의 GameplayTag Key와 일치시켜 조회합니다.
- **데미지 적용**: 각 GA는 `GE_Damage` 스펙을 만들고 `SetByCaller`로 `BossData->AbilityDamageMap[AbilityTag]` 값을 주입합니다.
- **Grant 경로**: 보스 캐릭터의 데이터 주입 시점(`SetData`/`BeginPlay`)에 해당 보스 전용 패턴 데이터의 `GrantedAbility`를 순회하여 ASC에 `GiveAbility`합니다(Ravager는 `BossPatternData` 맵 순회).
- **Phase 전이(Ravager)**: `ABORavagerBoss::OnDamageReceived` → `DetermineTargetPhase(HealthRatio)`가 목표 `EBOBossPhase`를 결정 → `ABlackoutRavagerAIController::RequestPhaseChange` → `UBlackoutPhaseEvaluator`(`Ability.PhaseLock` 게이팅) → `UBlackoutBossBTRunner`가 페이즈 BehaviorTree 교체. **StateTree Condition(`FBSTCond_HealthBelow`)은 Ravager 페이즈 전이에 사용되지 않습니다**(StateTree 빌딩 블록으로만 잔존).
- **기둥 파괴 경로**: Ravager 돌진/히트박스 계열 GA(`UBlackoutGA_Ravager_Charge` / `UBlackoutGA_Ravager_HitboxAttack`)가 만든 피해 스펙이 `ABOBreakablePillarActor::ReceiveDamageFromHitbox`에 전달되면 서버가 출처를 검증한 뒤 `BreakPillar()`를 호출합니다. 별도의 PillarCharge GA는 없습니다.
- **AI 호출 경로**: Ravager는 BehaviorTree 노드(`UBTT_ActivateAbility`)가, 미니언/Shrewd는 StateTree Task(`FBSTTask_ActivateAbility`)가 Ability Tag로 GA를 발동하고, GA는 `InstancedPerActor` 정책으로 각 인스턴스에서 실행됩니다.
