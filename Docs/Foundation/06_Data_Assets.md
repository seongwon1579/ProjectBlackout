# Foundation — 06. 데이터 에셋 베이스 (Data Assets)

> TDD v5 §11 참조. 모든 수치를 에셋화하여 기획자가 에디터에서 직접 조정 가능.
> 1차 구현 범위: **클래스 선언 + 필수 필드**. 실제 수치는 에셋 생성 후 채움.

```mermaid
classDiagram
    direction TB

    UPrimaryDataAsset <|-- UBOCharacterData
    UPrimaryDataAsset <|-- UBOMinionData
    UPrimaryDataAsset <|-- UBOBossData
    UDataTable ..> FBlackoutWeaponStat : row type

    class UBOCharacterData {
        <<PrimaryDataAsset>>
        +FGameplayTag ClassTag
        +float BaseMaxHealth
        +float BaseMaxStamina
        +int32 InitialBloodRoot
        +int32 InitialGulSerum
        +TSoftObjectPtr~UTexture2D~ PortraitIcon
        +TArray~TSubclassOf~UGameplayAbility~~ GrantedAbilities
        +TSubclassOf~AActor~ StartingPrimaryWeapon
        +TSubclassOf~AActor~ StartingSecondaryWeapon
    }

    class UBOMinionData {
        <<PrimaryDataAsset>>
        +float MaxHealth
        +float MovementSpeed
        +TMap~FGameplayTag, float~ AbilityDamageMap
    }

    class UBOBossData {
        <<PrimaryDataAsset>>
        +TArray~float~ PhaseHealthCutlines
        +TMap~FGameplayTag, float~ HitPartMultipliers
        +float AggroSwitchCooldown
        +float AggroDamageThreshold
        +float AggroDecayRate
        +TMap~FGameplayTag, float~ AbilityDamageMap
    }

    class FBlackoutWeaponStat {
        <<Struct — DT_WeaponStats 행>>
        +FGameplayTag WeaponTag
        +float BaseDamage
        +float FireRate
        +int32 MagazineSize
        +float SplashRadius
    }
```

## 에셋별 참조 위치

| 데이터 에셋 | 주요 참조처 |
|---|---|
| `UBOCharacterData` | `ABlackoutPlayerState::ApplyBattleTransitionPolicy`, `ABlackoutLobbyGameMode::PostLogin` GA 부여 |
| `UBOMinionData` | `ABlackoutEnemyCharacter::BeginPlay` 어트리뷰트 주입 |
| `UBOBossData` | `ABlackoutBossCharacter` 페이즈 컷라인, `UBlackoutAggroComponent` 튜닝 |
| `DT_WeaponStats` | `UBlackoutCombatComponent` 무기 스탯 조회 |

## 구현 노트

- 모든 에셋은 `Content/_BP/Core/Data/` 에 배치.
- `UBOBossData.AggroSwitchCooldown` 기본값 `5.0`, `AggroDamageThreshold` `0.15`, `AggroDecayRate` `0.02` (TDD §6.1).
- `UBOBossData.PhaseHealthCutlines`: Phase A→B 60%, B→C 30% 기준.
