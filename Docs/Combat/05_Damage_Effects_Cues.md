# Combat — 05. 데미지 이펙트 / 보상 / 큐 (Damage Effects & Cues)

> TDD v5 §5 참조. `GE_Damage` + `ExecCalc_DamageCalc`(공통기반 §03) + `ExecCalc_CombatReward` + GameplayCue.

```mermaid
classDiagram
    direction TB

    UGameplayEffect <|-- GE_Damage
    UGameplayEffect <|-- GE_Reload
    UGameplayEffectExecutionCalculation <|-- UExecCalc_DamageCalc
    UGameplayEffectExecutionCalculation <|-- UExecCalc_CombatReward
    UGameplayEffectExecutionCalculation <|-- UExecCalc_Reload

    UGameplayCueNotify_Static <|-- UGCN_Weapon_Fire
    UGameplayCueNotify_Static <|-- UGCN_Weapon_Reload
    UGameplayCueNotify_Static <|-- UGCN_HitImpact

    class GE_Damage {
        <<Instant>>
        +Executions : UExecCalc_DamageCalc
        +SetByCaller : Data.Damage
        +SetByCaller : Body.WeakSpot
        +SetByCaller : Body.ArmoredLimb
        +GameplayCues : GameplayCue.Character.Hit
    }

    class GE_Reload {
        <<Instant>>
        +Executions : UExecCalc_Reload
        +SetByCaller : Data.ReloadAmount
    }

    class UExecCalc_CombatReward {
        +Execute_Implementation(...) void
        +SelectedClassTag 검증
        +KillTag 검증
        +DropItem 1개 랜덤 선택
    }

    class UExecCalc_Reload {
        +Execute_Implementation(...) void
    }

    GE_Damage ..> UExecCalc_DamageCalc
    GE_Reload ..> UExecCalc_Reload
    UGCN_HitImpact ..> GE_Damage : Triggered by Cue Tag
```

## 구현 노트

- **`GE_Damage`**:
  - `DurationPolicy=Instant`. Source 기준 `Spec.SetSetByCallerMagnitude(Data.Damage, ...)` 로 기본값 세팅 → GA에서 주입.
  - 히트박스 부위 태그는 `ABOProjectile::OnHit` 또는 `UBlackoutGA_FireWeapon`이 SpecHandle에 `SetByCaller`로 주입.
  - Cue 태그 `GameplayCue.Character.Hit` 로 `UGCN_HitImpact [Static]` 트리거.
- **`ExecCalc_CombatReward` (TDD §5.1)**:
  - 서버 사망 확정 경로는 직접 드롭을 생성하지 않고, `UBlackoutCombatRewardSettings::CombatRewardEffectClass`에 지정된 `GE_CombatReward`를 적용합니다.
  - `GE_CombatReward`는 Instant GE이며, Execution에 BP 서브클래스 `ExecCalc_CombatReward`를 지정합니다. 드롭 아이템 액터 클래스는 캐릭터 BP가 아니라 이 BP ExecCalc의 `DropItemClass` 기본값에서 지정합니다.
  - Source PlayerState의 `SelectedClassTag` + 마지막 타격 Spec의 킬 태그 조합을 검증합니다.
  - 로비 선택을 거치지 않는 테스트 맵에서는 Source 플레이어 캐릭터의 `CharacterData.ClassTag`를 병과 태그 폴백으로 사용합니다.
  - 조건 만족 시 즉시 자원을 지급하지 않고, 일반 적(`ABlackoutEnemyCharacter`, 보스 제외) 사망 위치 주변 후보점을 바닥 트레이스로 보정해 `ABlackoutDropItem` 1개를 `UBlackoutPoolSubsystem::SpawnFromPool`로 스폰합니다. 이후 드롭 액터가 `PickupMesh` bounds 기준으로 한 번 더 바닥 스냅합니다.
  - 드롭 후보는 주무기 탄약 40% / 보조무기 탄약 40% / 소모품 20% 중 하나이며, 실제 탄약·소모품 지급은 플레이어가 `[E]` 상호작용으로 획득할 때 처리합니다.
  - 클래스 A: `Kill.Melee`, 클래스 B: `Kill.MultiTarget.Count3`, 클래스 C: `Kill.WeakSpot`.
  - 실행 시점은 `ExecCalc_DamageCalc`와 같은 GE 실행 순서에 의존하지 않고, 서버에서 데미지 적용 후 치명 피해가 확정된 직후이자 `OnDeath()` 사이드 이펙트 실행 전의 사망 처리 경로에서 `GE_CombatReward`를 적용합니다. 다중 처치는 산탄·폭발·스플래시 판정 소유자가 배치 내 처치 수를 집계한 뒤 `Kill.MultiTarget.Count3`가 포함된 보상 Spec으로 후처리합니다.
- **`ExecCalc_Reload`**:
  - `Missing = MaxClip - ClipAmmo`, `Grant = min(Missing, ReserveAmmo)` → `ClipAmmo += Grant`, `ReserveAmmo -= Grant`.
  - 주/보조 구분은 Spec의 `InstigatorTags`로 전달받음.
- **GameplayCue 분리 (TDD §11)**:
  - `Static` — 일회성(총구 화염, 탄창 탈착음, 혈흔). 객체 인스턴스 없음.
  - `Actor` — 지속형(아직 전투 에픽에는 없음, 보스 Red Mist 등 AI 에픽에서 사용).
- **무기별 Cue 세트**: 발사, 탄 궤적, 표면 재질별 Impact Cue는 무기 데이터의 `FBlackoutWeaponCueSet`에서 태그로 지정하고 서버 ASC에서 실행합니다. 상세 클래스 다이어그램은 [10_Weapon_GameplayCue_Set.md](10_Weapon_GameplayCue_Set.md)를 참조합니다.
