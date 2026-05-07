# Foundation — 08. 전체 의존 관계 개요 (Dependency Overview)

> 공통 기반 레이어 내부의 의존 흐름. 개별 에픽(Combat / AI-Boss / UI / Lobby / Battle)은 이 그래프 위에 얹혀 성장.

```mermaid
flowchart TB
    subgraph Utils["07. Utilities / Tags / Logging"]
        Tags[BlackoutGameplayTags]
        Logs[BlackoutLogCategories]
        Enum[EBlackoutAbilityInputID\nEBattleTransitionType]
    end

    subgraph Iface["04. Common Interfaces"]
        IPool[IBlackoutPoolableInterface]
        IInt[IBlackoutInteractable]
        IDmg[IBlackoutDamageable]
    end

    subgraph Data["06. Data Assets"]
        CData[UBOCharacterData]
        MData[UBOMinionData]
        BData[UBOBossData]
        WStat[FBlackoutWeaponStat / DT_WeaponStats]
        CnsData[UBOConsumableData]
    end

    subgraph GAS["03. GAS Infrastructure"]
        ASC[UBlackoutAbilitySystemComponent]
        Attr[UBlackoutBaseAttributeSet]
        GA[UBlackoutGameplayAbility]
        GEBase[GE_Damage\nGE_Downed\nGE_BleedOut]
        Calc[UExecCalc_DamageCalc]
    end

    subgraph Character["02. Character Hierarchy"]
        CBase[ABlackoutCharacterBase]
        PCh[ABlackoutPlayerCharacter]
        ECh[ABlackoutEnemyCharacter]
    end

    subgraph Framework["01. Framework Core"]
        GM[ABlackoutGameMode]
        GS[ABlackoutGameState]
        PS[ABlackoutPlayerState]
        PC[ABlackoutPlayerController]
    end

    subgraph Pool["05. Pooling Subsystem"]
        PoolSys[UBlackoutPoolSubsystem]
    end

    %% 의존 방향
    GAS --> Utils
    Character --> GAS
    Character --> Iface
    Framework --> GAS
    Framework --> Data
    Framework --> Character
    Pool --> Iface
    GS --> Tags
    GEBase --> Calc
    Calc --> Attr
    Calc --> Tags
    GA --> Enum

    classDef layer fill:#2d3748,stroke:#4299e1,color:#e2e8f0
    class Tags,Logs,Enum,IPool,IInt,IDmg,CData,MData,BData,WStat,CnsData,ASC,Attr,GA,GEBase,Calc,CBase,PCh,ECh,GM,GS,PS,PC,PoolSys layer
```

## 권장 구현 순서

| 단계 | 대상 | 이유 |
|---|---|---|
| 1 | **§07 유틸·태그·로깅** | 의존성 0, 모든 레이어가 `#include` |
| 2 | **§04 공통 인터페이스** | 헤더만 선언, 풀링·캐릭터보다 먼저 필요 |
| 3 | **§06 데이터 에셋 스켈레톤** | Framework/GAS가 참조하기 전에 타입 확정 |
| 4 | **§03 GAS 인프라** | ASC/AttributeSet → GA/GE/ExecCalc 순 |
| 5 | **§02 캐릭터 베이스** | GAS 인터페이스 확정 후 `InitAbilityActorInfo` 가능 |
| 6 | **§01 프레임워크 코어** | PlayerState(ASC 소유) 포함, 상위 의존 마지막 |
| 7 | **§05 풀링 서브시스템** | `IBlackoutPoolableInterface` 확정 후 착수 |

> 각 단계 완료 시 `feature/foundation-step<N>` 브랜치로 PR → `develop` 머지.
