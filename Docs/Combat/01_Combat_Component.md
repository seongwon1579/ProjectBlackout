# Combat — 01. 전투 컴포넌트 (Combat Component)

> TDD v5 §2 "플레이어 전용 무기 행동 분리" 참조. 플레이어 캐릭터에만 부착, 조준/사격/장전/무기 스왑 등 입력-무기 중계 허브.

```mermaid
classDiagram
    direction TB

    UActorComponent <|-- UBlackoutCombatComponent

    class UBlackoutCombatComponent {
        <<ActorComponent>>
        -ABOFirearm* PrimaryWeapon
        -ABOFirearm* SecondaryWeapon
        -ABOMeleeWeapon* MeleeWeapon
        -ABOWeaponBase* EquippedWeapon
        -bool bIsAiming
        -float AimParallaxOffset
        +EquipPrimary() void
        +EquipSecondary() void
        +SwapWeapon() void
        +StartFire() void
        +StopFire() void
        +StartAim() void
        +StopAim() void
        +TryReload() void
        +PerformMeleeHit() void
        +GetMuzzleTransform() FTransform
        +Server_EquipWeapon(ABOWeaponBase*) void
        +OnRep_EquippedWeapon() void
    }

    UBlackoutCombatComponent o-- ABOWeaponBase : EquippedWeapon
    UBlackoutCombatComponent o-- ABOFirearm : Primary / Secondary
    UBlackoutCombatComponent o-- ABOMeleeWeapon
    ABlackoutPlayerCharacter *-- UBlackoutCombatComponent : (PlayerCharacter 전용)
```

## 구현 노트

- **부착 대상**: `ABlackoutPlayerCharacter`에만 부착. 적 캐릭터는 본 컴포넌트 없음.
- **착탄 인디케이터 책임 분리**: 카메라 조준 대상, 총구 기준 실제 착탄, 투사체 예측 착탄 계산은 `UBlackoutImpactIndicatorComponent`가 담당합니다. 본 컴포넌트는 현재 장착 무기, 조준 상태, 총구 Transform 등 전투 상태를 제공하는 쪽으로 제한합니다.
- **무기 스왑**: `Server_EquipWeapon` RPC → `EquippedWeapon` replicated → `OnRep_EquippedWeapon`에서 3인칭 모델 attach 처리.
- **GAS 연결**: 입력(StartFire/TryReload 등)은 대응 `UBlackoutGameplayAbility`를 `TryActivateAbilityByClass`로 기동. 비용·쿨다운·Cost 체크는 GA 레이어에서 담당(본 컴포넌트는 상태 미보유).
- **조준 상태**: `bIsAiming` 는 GA에서도 조회 가능하도록 `Replicated`. IK/Aim Offset BP에서 바인딩.
