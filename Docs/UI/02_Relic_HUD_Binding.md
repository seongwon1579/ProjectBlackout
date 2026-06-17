# UI — 02. 유물 HUD 바인딩

> GDD §3 유물 회복 시스템, §8.3 인게임 전투 HUD, 클래스 다이어그램 §3~§6 기반.
> 유물 남은 개수는 `UBlackoutPlayerAttributeSet`의 GAS Attribute로 관리하고, HUD는 Attribute Delegate 이벤트만 받아 표시합니다.

```mermaid
classDiagram
    direction TB

    IAbilitySystemInterface <|.. ABlackoutPlayerState
    UAttributeSet <|-- UBlackoutPlayerAttributeSet
    UBlackoutGameplayAbility <|-- UBlackoutGA_UseRelic
    UBlackoutGameplayAbility <|-- UBlackoutGA_Revive
    UUserWidget <|-- UBlackoutHUDWidget
    UUserWidget <|-- UBlackoutRelicWidget
    UObject <|-- UBlackoutHUDWidgetController

    class ABlackoutPlayerState {
        -UBlackoutAbilitySystemComponent* ASC
        -UBlackoutPlayerAttributeSet* PlayerAttributeSet
        +GetAbilitySystemComponent() UAbilitySystemComponent*
        +ApplyBattleTransitionPolicy(EBattleTransitionType) void
    }

    class UBlackoutPlayerAttributeSet {
        +FGameplayAttributeData RelicCharges
        +FGameplayAttributeData MaxRelicCharges
        +OnRep_RelicCharges(FGameplayAttributeData) void
        +OnRep_MaxRelicCharges(FGameplayAttributeData) void
        +PreAttributeChange(FGameplayAttribute, float&) void
        +PostGameplayEffectExecute(FGameplayEffectModCallbackData) void
    }

    class UBlackoutGA_UseRelic {
        +CanActivateAbility(...) bool
        +ActivateAbility(...) void
        -CommitRelicCost() bool
        -ApplyRelicHeal() void
        -PlayLockInMontage() void
    }

    class UBlackoutGA_Revive {
        +CanActivateAbility(...) bool
        +ActivateAbility(...) void
        -StartHoldToRevive() void
        -CommitRescuerRelicCost() bool
        -ClearDownedState() void
    }

    class UBlackoutHUDWidgetController {
        -TWeakObjectPtr~ABlackoutPlayerState~ PlayerState
        -TWeakObjectPtr~UAbilitySystemComponent~ AbilitySystemComponent
        -TWeakObjectPtr~UBlackoutPlayerAttributeSet~ PlayerAttributeSet
        +Initialize(APlayerController*) void
        +BindCallbacksToDependencies() void
        +BroadcastInitialValues() void
        +OnRelicChargesChanged(int32 Current, int32 Max)
        -HandleRelicChargesChanged(FOnAttributeChangeData) void
        -HandleMaxRelicChargesChanged(FOnAttributeChangeData) void
        -BroadcastRelicCharges() void
    }

    class UBlackoutHUDWidget {
        -UBlackoutRelicWidget* RelicWidget
        +SetWidgetController(UBlackoutHUDWidgetController*) void
        #ReceiveRelicChargesChanged(int32 Current, int32 Max) void
    }

    class UBlackoutRelicWidget {
        -int32 CurrentCharges
        -int32 MaxCharges
        -TArray~UWidget*~ ChargePips
        +SetRelicCharges(int32 Current, int32 Max) void
        -RebuildChargePips() void
        -UpdateChargeStates() void
    }

    class GE_RelicHeal {
        <<GameplayEffect>>
        +Health 회복
    }

    class GE_RelicCost {
        <<GameplayEffect>>
        +RelicCharges -1
    }

    class EBattleTransitionType {
        <<Enum>>
        LobbyToBattle
        CheckpointRest
        PartyWipeRespawn
    }

    ABlackoutPlayerState o-- UBlackoutAbilitySystemComponent : owns
    ABlackoutPlayerState o-- UBlackoutPlayerAttributeSet : has
    UBlackoutAbilitySystemComponent --> UBlackoutPlayerAttributeSet : replicates attributes
    UBlackoutGA_UseRelic --> GE_RelicCost : applies cost
    UBlackoutGA_UseRelic --> GE_RelicHeal : applies heal
    UBlackoutGA_Revive --> GE_RelicCost : rescuer cost
    ABlackoutPlayerState --> UBlackoutPlayerAttributeSet : transition reset to max
    UBlackoutHUDWidgetController --> ABlackoutPlayerState : resolves ASC owner
    UBlackoutHUDWidgetController --> UBlackoutPlayerAttributeSet : reads Relic attributes
    UBlackoutHUDWidgetController --> UBlackoutHUDWidget : broadcasts display event
    UBlackoutHUDWidget o-- UBlackoutRelicWidget : owns
    UBlackoutHUDWidget --> UBlackoutRelicWidget : forwards Current/Max
```

## 바인딩 흐름

```mermaid
sequenceDiagram
    autonumber
    participant PC as ABlackoutPlayerController
    participant WC as UBlackoutHUDWidgetController
    participant PS as ABlackoutPlayerState
    participant ASC as UAbilitySystemComponent
    participant Attr as UBlackoutPlayerAttributeSet
    participant HUD as UBlackoutHUDWidget
    participant Relic as UBlackoutRelicWidget

    PC->>WC: Initialize(PC)
    WC->>PS: GetPlayerState()
    WC->>ASC: PS.GetAbilitySystemComponent()
    WC->>Attr: Resolve PlayerAttributeSet
    WC->>ASC: GetGameplayAttributeValueChangeDelegate(RelicCharges)
    WC->>ASC: GetGameplayAttributeValueChangeDelegate(MaxRelicCharges)
    WC->>WC: BroadcastInitialValues()
    WC->>HUD: OnRelicChargesChanged(Current, Max)
    HUD->>Relic: SetRelicCharges(Current, Max)
    ASC-->>WC: RelicCharges / MaxRelicCharges changed
    WC->>WC: BroadcastRelicCharges()
    WC->>HUD: OnRelicChargesChanged(Current, Max)
    HUD->>Relic: SetRelicCharges(Current, Max)
```

## 유물 수량 변경 경로

```mermaid
flowchart TB
    UseRelic["UBlackoutGA_UseRelic"] --> Cost["GE_RelicCost: RelicCharges -1"]
    Revive["UBlackoutGA_Revive"] --> Cost
    Transition["ABlackoutPlayerState::ApplyBattleTransitionPolicy"] --> Reset["RelicCharges = MaxRelicCharges"]
    Cost --> Attr["UBlackoutPlayerAttributeSet"]
    Reset --> Attr
    Attr --> Delegate["ASC Attribute Change Delegate"]
    Delegate --> Controller["UBlackoutHUDWidgetController"]
    Controller --> HUD["UBlackoutHUDWidget"]
    HUD --> RelicWidget["UBlackoutRelicWidget"]
```

## 구현 노트

- **수량 소유**: 유물 현재/최대 수량은 `ABlackoutPlayerState`가 보유한 ASC의 `UBlackoutPlayerAttributeSet`에 둡니다. 소모품처럼 별도 Replicated int 프로퍼티를 만들지 않습니다.
- **표시 갱신**: `UBlackoutHUDWidgetController`는 `RelicCharges`, `MaxRelicCharges` Attribute Delegate를 바인딩하고, 변경 시 `int32 Current`, `int32 Max`로 변환해 HUD에 전달합니다.
- **위젯 책임**: `UBlackoutRelicWidget`은 전달받은 수량으로 아이콘 또는 pip 상태만 갱신합니다. ASC, AttributeSet, GA를 직접 조회하지 않습니다.
- **초기 표시**: HUD 생성 직후 `BroadcastInitialValues()`에서 현재 유물 수량을 즉시 전달해 첫 프레임 빈 슬롯을 방지합니다.
- **클램프 정책**: `RelicCharges`는 `0 ~ MaxRelicCharges` 범위로 제한합니다. 기본/최대값은 GDD 기준 `3`입니다.
- **리셋 경로**: 전투 맵 진입, 화톳불 휴식, 전멸 후 체크포인트 복귀 정책에서는 `ABlackoutPlayerState::ApplyBattleTransitionPolicy`가 유물을 최대치로 회복시킵니다.
- **실패 처리**: 유물 사용 또는 부활 시 `RelicCharges <= 0`이면 GA 활성화를 실패시키고 원인 파악 가능한 로그를 남깁니다.
