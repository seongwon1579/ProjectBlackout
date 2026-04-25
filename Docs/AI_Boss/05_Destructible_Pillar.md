# AI/Boss — 05. 파괴 가능 기둥 (Destructible Pillar)

> TDD v5 §8 참조. Chaos Destruction(Geometry Collection)으로 **Ravager의 돌진 계열 GA(`UGA_Ravager_LungeAttackCombo`, `UGA_Ravager_ChargedShockwave`)의 히트 부차 효과로** 파괴.
> **2026-04-21 GDD 개정**: GDD §6 "보스의 돌진 공격에 의해 영구적으로 파괴"를 반영하여 별도의 `GA_Ravager_PillarCharge`를 제거하고, 돌진 계열 GA 히트 판정에 통합.

```mermaid
classDiagram
    direction TB

    AActor <|-- ABlackoutDestructiblePillar
    ABlackoutDestructiblePillar ..|> IBlackoutDamageable : implements

    class ABlackoutDestructiblePillar {
        <<AActor>>
        -UGeometryCollectionComponent* GCComponent
        -UStaticMeshComponent* IntactMesh
        -UBoxComponent* BlockingCollision
        -float BOPillarHealth
        -bool bIsShattered
        -int32 PillarId
        +BeginPlay() override void
        +TakeDamage_FromCharge(float Amount, FVector Impact, FVector Force) void
        +Multicast_Shatter(FVector ImpactPoint, FVector ImpactForce) void
        +Instant_Shatter() void
        +OnRep_IsShattered() void
        #SettleDebris() void
    }

    class IBlackoutDamageable {
        <<Interface>>
        +ReceiveDamage(float, AActor* Instigator) void
    }

    class UGeometryCollectionComponent {
        <<Chaos — 런타임 분할 시뮬레이션>>
    }

    ABlackoutDestructiblePillar *-- UGeometryCollectionComponent
    ABlackoutDestructiblePillar o-- ABlackoutGameState : reports PillarId to DestroyedPillarIds
    UGA_Ravager_LungeAttackCombo ..> ABlackoutDestructiblePillar : 히트 시 Multicast_Shatter
    UGA_Ravager_ChargedShockwave ..> ABlackoutDestructiblePillar : 히트 시 Multicast_Shatter
```

## 파괴 플로우

```mermaid
sequenceDiagram
    participant Ravager as ABORavagerBoss
    participant GA as GA_Ravager_LungeAttackCombo<br/>or GA_Ravager_ChargedShockwave
    participant Pillar as ABlackoutDestructiblePillar
    participant GS as ABlackoutGameState

    Ravager->>GA: BT Activate (돌진 계열 GA)
    GA->>Pillar: Hit Trace (bBreaksPillarOnHit=true) → TakeDamage_FromCharge
    Pillar->>Pillar: BOPillarHealth -= Amount
    alt HP <= 0 (Server)
        Pillar->>Pillar: bIsShattered = true (Replicated)
        Pillar->>Pillar: Multicast_Shatter(ImpactPoint, ImpactForce)
        Pillar-->>GS: DestroyedPillarIds.AddUnique(PillarId)
    end
    Note over Pillar: Client: GC ApplyExternalStrain() → 시뮬레이션
    Pillar->>Pillar: SettleDebris() (5s 지연 후 SetSimulatePhysics=false)
```

## 구현 노트

- **결정적 시뮬레이션 아님**: Chaos 시뮬레이션은 클라이언트별로 독립 실행. 서버는 `bIsShattered` 플래그만 리플리케이트하여 대역폭 절감.
- **Late-join / 관전자**: 신규 접속자는 `OnRep_IsShattered`에서 `Instant_Shatter()` 호출 → 시뮬레이션 없이 즉시 잔해 상태 스폰.
- **피해 필터**: `TakeDamage_FromCharge`는 **돌진 계열 GA**(`UGA_Ravager_LungeAttackCombo`, `UGA_Ravager_ChargedShockwave`) 인스티게이터만 수용. 각 GA 스펙의 `SetByCaller(Pillar.Damage)` 값과 `bBreaksPillarOnHit` 플래그를 확인하여 판정. 플레이어 총격/폭발·다른 GA 히트는 `IBlackoutDamageable::ReceiveDamage`에서 무시.
- **성능 가드**: `SettleDebris()`는 5초 뒤 물리 비활성화. 추가로 8초 경과 잔해는 서버 Tick에서 `SetActorHiddenInGame(true)`.
- **`ABlackoutGameState::DestroyedPillarIds`**: Phase C 진입 시 Ravager가 이 배열 길이를 참조하여 회피 난이도 로직 (예: 카메라 세이프티 영역 축소)에 반영.
