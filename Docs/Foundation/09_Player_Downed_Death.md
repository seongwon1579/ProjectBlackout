# Foundation — 09. 플레이어 다운 / 완전 사망

> HP 0 도달 시 즉시 매치 실패로 처리하지 않고, `State.Downed` 상태에서 부활 가능 시간을 제공한 뒤 타이머가 만료되면 완전 사망으로 전환하는 책임 구조입니다.
> 현재 코드에는 `State.Downed`, `State.Reviving`, `State.BeingRevived`와 `GA_Revive`가 존재하며, 본 문서는 후속 구현 대상인 완전 사망 타이머와 전멸 감지 훅을 함께 정의합니다.

## 클래스 다이어그램

```mermaid
classDiagram
    direction TB

    ABlackoutCharacterBase <|-- ABlackoutPlayerCharacter
    UBlackoutGameplayAbility <|-- UBlackoutGA_Revive
    ABlackoutGameMode <|-- ABlackoutBattleGameMode

    class ABlackoutCharacterBase {
        <<Abstract>>
        -bool bIsDead
        -bool bReplicatedDownedStateTag
        +IsDead() bool
        +IsDowned() bool
        #ApplyIncomingDamageSpec(FGameplayEffectSpecHandle, FName) bool
        #OnDowned() void
        #OnDeath() void
        #CanEnterDownedState() bool
        #SetDownedStateActive(bool) void
        #SetDeadStateActive(bool) void
    }

    class ABlackoutPlayerCharacter {
        -bool bIsReviveInteractionActive
        -FTimerHandle DownedDeathTimerHandle
        -float DownedDeathDuration
        -TWeakObjectPtr~ABlackoutPlayerCharacter~ ActiveReviver
        +IsReviving() bool
        +IsBeingRevived() bool
        +TryBeginReviveInteraction(ABlackoutPlayerCharacter*) bool
        +EndReviveInteraction(ABlackoutPlayerCharacter*) void
        +Server_ReviveFromDowned(float) void
        #OnDowned() void
        #OnDeath() void
        #CanEnterDownedState() bool
        -StartDownedDeathTimer() void
        -ClearDownedDeathTimer() void
        -HandleDownedDeathTimerExpired() void
        +RestoreFromPartyWipeRestart() void
    }

    class UBlackoutGA_Revive {
        -float ReviveDuration
        -float ReviveTickInterval
        -float ReviveRange
        -float RevivedHealthPercent
        -FTimerHandle ReviveTickTimerHandle
        -TObjectPtr~ABlackoutPlayerCharacter~ CachedReviver
        -TObjectPtr~ABlackoutPlayerCharacter~ CachedTarget
        +ActivateAbility(...) void
        +InputReleased(...) void
        +EndAbility(...) void
        -HandleReviveTick() void
        -FinishRevive() void
        -CancelRevive() void
        -CanReviveTarget(...) bool
    }

    class UBlackoutAbilitySystemComponent {
        +AddLooseGameplayTag(FGameplayTag) void
        +RemoveLooseGameplayTag(FGameplayTag) void
        +CancelHealthRegenOverTime() void
        +CancelAbilities(...) void
    }

    class UBlackoutBaseAttributeSet {
        +FGameplayAttributeData Health
        +FGameplayAttributeData MaxHealth
        +PostGameplayEffectExecute(FGameplayEffectModCallbackData) void
    }

    class UBlackoutPlayerAttributeSet {
        +FGameplayAttributeData RelicCharges
        +FGameplayAttributeData MaxRelicCharges
    }

    class ABlackoutBattleGameMode {
        +HandlePartyWipe() void
        +NotifyPlayerFullyDead(ABlackoutPlayerCharacter*) void
        -EvaluatePartyWipe() void
    }

    class ABlackoutPlayerState {
        +IsDowned() bool
        +IsReviving() bool
        +IsBeingRevived() bool
        +ApplyBattleTransitionPolicy(EBattleTransitionType) void
    }

    class ABlackoutGameState {
        +TArray~APlayerState*~ PlayerArray
        +SetMatchState(EBlackoutMatchState) void
    }

    class BlackoutGameplayTags {
        <<Native Tags>>
        +State_Downed
        +State_Reviving
        +State_BeingRevived
        +State_Dead
    }

    ABlackoutCharacterBase --> UBlackoutAbilitySystemComponent : owns/resolves ASC
    ABlackoutCharacterBase --> UBlackoutBaseAttributeSet : reads Health
    ABlackoutCharacterBase --> BlackoutGameplayTags : State.Downed
    ABlackoutPlayerCharacter --> BlackoutGameplayTags : State.Dead / State.BeingRevived
    ABlackoutPlayerCharacter --> UBlackoutGA_Revive : revive target
    UBlackoutGA_Revive --> UBlackoutPlayerAttributeSet : consumes RelicCharges
    UBlackoutGA_Revive --> UBlackoutBaseAttributeSet : restores Health
    ABlackoutPlayerCharacter --> ABlackoutBattleGameMode : NotifyPlayerFullyDead
    ABlackoutBattleGameMode --> ABlackoutGameState : PlayerArray / MatchState
    ABlackoutBattleGameMode --> ABlackoutPlayerState : ApplyBattleTransitionPolicy
```

## 상태 전환 책임

| 상태 | 원천 | 진입 | 이탈 |
|---|---|---|---|
| Alive | `Health > 0`, `!State.Downed`, `!State.Dead` | 부활 완료, 전멸 복귀 | HP 0 |
| Downed | `State.Downed` | `ABlackoutCharacterBase::ApplyIncomingDamageSpec` → `OnDowned` | 부활 완료 또는 다운 사망 타이머 만료 |
| BeingRevived | `State.BeingRevived` | `TryBeginReviveInteraction` 성공 | `EndReviveInteraction`, 부활 완료, 완전 사망 |
| Reviving | `State.Reviving` | 구출자가 부활 진행 시작 | 부활 완료, 입력 해제, 피격 캔슬, 대상 완전 사망 |
| Dead | `State.Dead` + `bIsDead` | `HandleDownedDeathTimerExpired` → `OnDeath` | 전멸 복귀 / 재시작 정책 |

## 구현 노트

- **HP 0 분기**: 기존 `ABlackoutCharacterBase::ApplyIncomingDamageSpec`가 HP 0 도달을 감지하고, 플레이어는 `CanEnterDownedState()`를 통해 `OnDowned()`로 진입합니다.
- **다운 타이머 소유자**: 플레이어 전용 타이머는 `ABlackoutPlayerCharacter`가 소유합니다. 다운 진입 시 서버에서 `StartDownedDeathTimer()`, 부활 완료 또는 전멸 복귀 시 `ClearDownedDeathTimer()`를 호출합니다.
- **완전 사망 표현**: 후속 구현에서는 `State.Dead` 네이티브 태그를 추가해 GA 차단, UI 표시, 파티 전멸 평가의 공통 판정값으로 사용합니다. `bIsDead`는 서버 중복 처리 가드로 유지합니다.
- **GA 차단**: `UBlackoutGameplayAbility` 기본 차단 태그에 `State.Dead`를 넣어 플레이어/AI 공통으로 완전 사망 후 새 어빌리티 시작을 막습니다.
- **부활과 타이머 우선순위**: 부활 진행 중이어도 다운 사망 타이머는 계속 흐릅니다. 타이머 만료가 먼저 도달하면 `GA_Revive`를 캔슬하고 대상은 완전 사망합니다.
- **전멸 감지**: 완전 사망 전환이 발생한 서버 경로에서 `ABlackoutBattleGameMode::NotifyPlayerFullyDead`를 호출하고, `EvaluatePartyWipe()`가 `GameState->PlayerArray` 기준으로 생존자를 계산합니다.
- **전멸 복귀**: 생존자가 0명이면 기존 `HandlePartyWipe()` 경로를 재사용합니다. 이 경로는 체크포인트 텔레포트, `ApplyBattleTransitionPolicy(PartyWipeRestart)`, Ready 리셋, `InCombatReady` 복귀를 담당합니다.

## 구현 대상 파일

| 책임 | 파일 |
|---|---|
| `State.Dead` 태그 추가 | `Source/ProjectBlackout/GameplayTags/BlackoutGameplayTags.h/.cpp` |
| 다운 사망 타이머, 완전 사망 전환 | `Source/ProjectBlackout/Characters/BlackoutPlayerCharacter.h/.cpp` |
| 공통 사망/다운 가드 | `Source/ProjectBlackout/Characters/BlackoutCharacterBase.h/.cpp` |
| 완전 사망 중 GA 활성 차단 | `Source/ProjectBlackout/GAS/Abilities/BlackoutGameplayAbility.cpp` |
| 부활 진행 중 대상 사망 처리 | `Source/ProjectBlackout/GAS/Abilities/Player/BlackoutGA_Revive.h/.cpp` |
| 완전 사망 감지 후 전멸 평가 | `Source/ProjectBlackout/Framework/BlackoutBattleGameMode.h/.cpp` |
| 파티원 사망 표시 | `Source/ProjectBlackout/UI/BlackoutPartyRosterWidgetController.h/.cpp` |
