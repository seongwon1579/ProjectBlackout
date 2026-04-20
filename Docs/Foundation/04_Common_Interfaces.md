# Foundation — 04. 공통 인터페이스 (Common Interfaces)

> 에픽 간 결합도를 낮추는 공유 인터페이스. 헤더 선언만으로 의존성 없이 참조 가능.

```mermaid
classDiagram
    direction LR

    class IBlackoutPoolableInterface {
        <<Interface>>
        +OnSpawnFromPool() void
        +OnReturnToPool() void
        +GetPoolClass() TSubclassOf~AActor~
    }

    class IBlackoutInteractable {
        <<Interface>>
        +CanInteract(AActor* Instigator) bool
        +OnInteract(AActor* Instigator) void
        +GetInteractionPrompt() FText
    }

    class IBlackoutDamageable {
        <<Interface>>
        +GetHitPartTag(FName BoneName) FGameplayTag
        +ReceiveDamageFromHitbox(FGameplayEffectSpec, FName BoneName) void
    }

    ABlackoutEnemyCharacter ..|> IBlackoutPoolableInterface : implements
    ABlackoutBossCharacter ..|> IBlackoutDamageable : implements
    ABlackoutBonfire ..|> IBlackoutInteractable : implements
    ABlackoutPortal ..|> IBlackoutInteractable : implements
    ADropItem ..|> IBlackoutInteractable : implements
    ADropItem ..|> IBlackoutPoolableInterface : implements
```

## 구현 노트

| 인터페이스 | 구현 대상 | 용도 |
|---|---|---|
| `IBlackoutPoolableInterface` | 미니언 / 발사체 / 드랍 아이템 / 씨앗 포드 | 풀 서브시스템에서 Get/Return 시 호출 |
| `IBlackoutInteractable` | 화톳불 / 포털 / 드랍 아이템 | `[E]` 상호작용 월드 위젯·입력 공용 경로 |
| `IBlackoutDamageable` | 보스 히트박스 컴포넌트 부착 액터 | 부위별 Bone → `FGameplayTag` 배율 분기 (TDD §5.2) |

- `IBlackoutPoolableInterface`는 `UBlackoutPoolSubsystem`이 캐스팅 없이 호출하는 유일한 계약.
- `IBlackoutDamageable`은 보스 에픽에서 먼저 구현되지만, 인터페이스 선언 자체는 공통 기반에 포함.
