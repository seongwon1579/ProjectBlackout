# 프로젝트 블랙아웃 - 기술 설계서 (TDD) v5 (GDD 정합성 반영)

## 0. 네이밍 규칙 (Naming Convention)
- **접두사 전략**: 기본 접두사는 **`Blackout`** 을 사용합니다.
- **파일명 길이 제한(OS 기준 255자 / 엔진 빌드 캐시 안전치 권장)** 에 걸릴 경우에 한해 **`BO`** 로 축약해 사용합니다. (예: `UBlackoutBossData` → 경로가 길어지는 데이터 에셋은 `UBOBossData`)
- **GA/GE/GCN 등 GAS 오브젝트**는 관례상 축약형(`BO`)보다 **기능 네이밍**을 우선합니다(`GA_Ravager_Gorenado`). 접두사는 C++ 클래스와 UObject 파생 에셋에 적용합니다.

## 1. 프레임워크 및 네트워크 아키텍처
Unreal Engine **5.7.4 바이너리 빌드(버전 고정)** 기반의 **Dedicated Server(데디케이티드 서버) 전용** 구조를 채택합니다. *(Listen Server 미지원)* 프로토타입 단계에서는 고정 IP 직접 접속으로 테스트하며, 정식 서비스 전환 시 AWS 상에 전용 매치메이킹 서버를 별도 구축합니다. 게임 플레이 로직 전반에 **GAS(Gameplay Ability System)** 와 **데이터 기반 설계(Data-Driven Design)** 를 적용하여 모듈화 및 확장성을 극대화합니다.

- **GameMode 계층 구조**: 공통 로직은 부모 클래스에 집중하고, 레벨별 고유 로직은 자식 클래스로 분리하는 **상속 계층** 구조를 채택합니다.
  - **`ABlackoutGameMode` (부모)**: 서버 전용 베이스 클래스. 공통 기능(파티 전멸 판정, PlayerState 초기화, Server RPC 공통 처리) 담당.
  - **`ABlackoutLobbyGameMode` : `ABlackoutGameMode`**: 로비 레벨 전용. 4인 캐릭터 선택 상태 수집, Ready Check 대기, 전투 맵으로의 `ServerTravel` 트리거 담당.
  - **`ABlackoutBattleGameMode` : `ABlackoutGameMode`**: 전투 레벨 전용. 중간 보스 제압 → 메인 보스 맵 순환, 파티 전멸 판정, 화톳불 리스폰 처리, 승리 시 메인 메뉴 복귀 담당.
- **GameState (`ABlackoutGameState`)**: 매치 타이머, 전역 기믹(파괴된 기둥 상태 배열), 붉은 안개 페이즈 전환 여부 등을 동기화합니다.
- **PlayerState (`ABlackoutPlayerState`)**: AbilitySystemComponent(ASC)를 소유하며, 게임 내내 유지되어야 하는 자원 데이터를 보관 및 **서버와 완전 동기화**합니다.
  - **ASC 소유 어트리뷰트**: HP, Stamina, 탄약 4종, 유물 충전 횟수
  - **Replicated 프로퍼티**: 선택한 병과(`SelectedClassTag`), 소모품 소지 수량(블러드 루트 / 굴 혈청), Ready 상태 플래그

## 2. 캐릭터 클래스 상속 구조 (Character Hierarchy)
개별 컴포넌트의 중복 코드를 제거하고, GAS 연동 및 상태 관리를 일관성 있게 통제하기 위해 `ACharacter`를 상속받은 커스텀 베이스 클래스 계층 구조를 설계합니다.

- **`ABlackoutCharacterBase`**: 프로젝트의 모든 캐릭터(플레이어, 몬스터, 보스)가 공통 상수 및 인터페이스를 상속받기 위한 최상위 베이스 클래스입니다.
  - `AbilitySystemComponent`에 접근하기 위한 `IAbilitySystemInterface`를 기본적으로 상속받아 `GetAbilitySystemComponent()` 함수를 일괄 구현합니다.
  - 피격, 사망(OnDeath), 기절 등 애니메이션 몽타주와 GAS 태그 연동을 위한 공통 가상 함수(Virtual Function)가 포함됩니다.
- **`ABlackoutPlayerCharacter`**: `ABlackoutCharacterBase`를 상속받는 플레이어블 캐릭터입니다.
  - TPS 카메라 구동에 필수적인 `UCameraComponent` 및 `USpringArmComponent`를 부착합니다.
  - 크로스헤어 정렬 및 조준 오차(Parallax) 보상을 위한 `UBlackoutCombatComponent`를 개별적으로 부착하여 장전, 사격 분기 등 플레이어 특화 무기 행동을 분리합니다.
  - `PossessedBy` 함수를 오버라이딩하여, 빙의될 때 `ABlackoutPlayerState`에 있는 ASC를 찾아 `InitAbilityActorInfo`로 초기화하는 구조를 갖습니다.
- **`ABlackoutEnemyCharacter`**: `ABlackoutCharacterBase`를 상속받는 AI 기믹용 적 캐릭터 베이스입니다.
  - 모델 자체에 ASC를 소유(`CreateDefaultSubobject`)하며, `BeginPlay` 시점에 자기 자신의 ASC를 초기화하고 기본 미니언 속성값(Attribute Asset)을 주입받습니다.
  - 풀링 서브시스템 재사용을 위해 `IBlackoutPoolableInterface`를 구현하며, 큐에서 뽑힐 때(`OnSpawnFromPool`) ASC 스탯을 리셋하는 기능과 사망 래그돌 처리 후 풀로 반환되는 로직이 내장됩니다.
- **`ABlackoutBossCharacter`**: `ABlackoutEnemyCharacter`에서 확장되어, 다단계 페이즈 트랜지션 로직이 특화된 거대 보스입니다.
  - 페이즈에 따른 패턴 변경 제어를 위해 보스 전용 데이터 에셋(`UBOBossData`)을 참조하여 체력 임계점을 틱마다 지속 감시합니다.
  - 부위별 독립적인 피격 판정(등 약점 1.5배, 단단한 다리 껍질 반감 등)을 제어하기 위해, 기본 캡슐 콜리전을 넘어 특정 뼈대(Bone)에 추가적인 충돌(Hitbox) 컴포넌트를 부착하여 데미지 수신을 분배하는 처리를 가집니다.
  - **어그로 관리**: `UBlackoutAggroComponent`(§6.1 참조)를 부착하여 타겟 선정 로직을 전담 처리합니다.

## 3. 어트리뷰트 세트 (Attribute Set) 설계
원작 렘넌트 2의 스탯 시스템을 벤치마킹하여 전투 공방 및 데미지 산출에 직관적으로 활용될 값을 정의합니다. 보스 몬스터는 회피용 스태미나가 필요하지 않으므로, AttributeSet을 역할군에 따라 철저히 분리하여 메모리를 최적화합니다.

- **`UBlackoutBaseAttributeSet`**: 플레이어, 몬스터, 보스가 모두 공통으로 가지는 스테이터스
  - `Health`, `MaxHealth`: 생명력.
  - `MovementSpeed`: 이동 속도 (보스 페이즈 전환 시 속도 업이나 플레이어 이속 둔화 디버프 등에 쓰임).
  - `BaseDamage`: 기본 데미지 배율.
  - `DamageReduction`: 피해 감소율. 장갑(방어력) 수치 기반으로 계산되어 최종 피격 데미지를 줄여줍니다.
- **`UBlackoutPlayerAttributeSet`**: 플레이어블 캐릭터 전용 스테이터스
  - `Stamina`, `MaxStamina`: 구르기(회피) 및 전력 질주 액션에 소비되는 체술 전용 자원.
  - `CriticalHitChance`: 치명타 발생 확률 (저격수 C캐릭터의 조건부 드랍 판정 및 딜 고점에 필수적인 값).
  - `CriticalHitMultiplier`: 치명타 적중 시의 데미지 배율.
  - `HealingEffectiveness`: 회복 효율. 유물(Dragon Heart) 사용 시나 도트힐(Blood Root) 효과가 들어올 때 실제 회복되는 양을 보정합니다.
  - **`RelicCharges`, `MaxRelicCharges`**: 유물 충전 횟수 (기본 3회 / 최대 3회 고정). GE로 소모/충전 처리.
- **`UBlackoutAmmoAttributeSet`**: 플레이어 전용, 무기 탄약 전담 세트 (순환 참조 방지를 위해 분리)
  - `PrimaryClipAmmo`, `PrimaryMaxClip`: 주무기 현재 장탄수 / 최대 장탄수.
  - `PrimaryReserveAmmo`: 주무기 예비 탄약.
  - `SecondaryClipAmmo`, `SecondaryMaxClip`: 보조무기 현재 장탄수 / 최대 장탄수.
  - `SecondaryReserveAmmo`: 보조무기 예비 탄약.
  - **설계 근거**: 탄약은 전투 중 실시간으로 변동하며 GE/ExecCalc로 조작 대상이 되므로 Attribute로 관리하는 것이 정석입니다. 주/보조 무기 각각 별도 풀이 필요하므로 스칼라 어트리뷰트를 4쌍(현재/최대/예비)으로 구성합니다.
  - **소모품(블러드 루트/굴 혈청)은 Attribute가 아닌 `ABlackoutPlayerState`의 Replicated 프로퍼티로 관리**합니다. (인벤토리성 소지량 + 무제한 상한 특성)

## 4. Gameplay Ability (GA) 액션 모듈화 및 부여(Grant)
기본 액션은 하드코딩을 피하고 `UGameplayAbility`로 분리하여 캔슬 연계, 상태 강제 해제, 네트워크 리플리케이션을 제어합니다.

- **ASC 소유권 (Ownership) 및 관리 주체**
  - **플레이어 계열**: `ABlackoutPlayerState`가 ASC를 소유합니다. (다운/부활 시 어트리뷰트와 GA가 소실되지 않도록 보존하기 위함)
  - **몬스터 계열(보스/미니언)**: 기본적으로 `ACharacter` 자체가 ASC를 소유합니다. 단, 미니언은 **오브젝트 풀링**되어 재사용되므로 풀에서 꺼내질 때(`OnSpawnFromPool`) 이전에 부여되었던 상태이상(GE), 쿨다운 태그 등을 전부 날리고(Clear), 어트리뷰트(HP)를 복구시키는 **명시적인 ASC 리셋 로직**이 반드시 수행되어야 합니다.
- **GA 부여(Granting) 로직**
  - 모든 GA는 오직 **서버(Server) 권한**으로만 부여됩니다.
  - 플레이어는 폰에 빙의될 때(서버 측 `PossessedBy` 시점)에 **`UBOCharacterData(DataAsset)` 에 명시된 `GrantedAbilities` 배열**을 순회하여 ASC에 일괄 주입(`GiveAbility`)합니다. 보스와 몬스터는 `BeginPlay` 시점에 자기 자신의 몬스터 데이터 에셋을 읽어 패턴 GA를 부여받습니다.

- **분리된 주요 GA 목록**:
  - `GA_Dodge`: 방향키 기반 회피 동작. 실행 시 몽타주와 함께 순간적인 I-Frame(무적) 이펙트를 부여.
  - `GA_Sprint`: 스태미나를 지속적으로 깎으며 이동속도 증가 상태 부여.
  - `GA_FireWeapon`: 기본 사격 (라인트레이스 스캔 혹은 Projectile 발사체 스폰). Cost로 `PrimaryClipAmmo` 또는 `SecondaryClipAmmo` 1 소모. 사격 시 `GCN_Weapon_Fire`를 통해 총구 화염 및 반동 사운드 출력.
  - `GA_Reload`: 탄창 재장전. 시전 시 장전 몽타주와 함께 `GCN_Weapon_Reload`를 호출하여 재장전 사운드(탄창 탈착음)를 동기화하고, 완료 시점에서 `PrimaryReserveAmmo` → `PrimaryClipAmmo` 로 이전(ExecCalc로 예비탄 차감 및 장탄 보충 동시 처리).
  - `GA_Melee_Player`: 플레이어 전용 근접 무기(강철 검 등) 타격 로직. 단축키 입력 시 근접 몽타주 재생 및 전방 공격 판정(Sweep) 생성, 콤보 연계 관리.
  - `GA_Revive`: 쓰러진 아군 구출. 완료 시 **구출을 수행한 플레이어의 `RelicCharges` 어트리뷰트 1회 차감** 로직 포함.
  - `GA_UseConsumable_BloodRoot`: 블러드 루트 사용. PlayerState의 블러드 루트 소지량 1 차감 후 `GE_HealOverTime` 부여.
  - `GA_UseRelic`: 유물(Dragon Heart) 사용. Lock-in 애니메이션 재생과 동시에 `GE_RelicHeal` 이펙트로 즉각 체력 회복. 완료 시 `RelicCharges` 1회 차감. 시전 중 `State.Locked` 태그로 이동 및 액션 봉쇄.
  - `GA_UseGulSerum`: 굴 혈청 사용. PlayerState의 굴 혈청 소지량 1 차감 후 `GE_GulSerumBuff` 이펙트를 부여하여 60초간 스태미나 소비 50% 감소.
- **미니언 패턴 GA**:
  - `GA_Minion_MeleeAttack`: 미니언(Root Hollow)의 박치기 등 기본 근접 공격. 타격 판정(Sweep, Overlap)에 맞춰 타겟에게 `GE_Damage` 부여.
  - `GA_Wraith_FireArrow`: 엘리트 미니언(Root Wraith)의 원거리 2연발 화살 투사체 발사. 풀비용(Cost) 없이 쿨다운만 적용.
  - `GA_Wraith_Teleport`: 엘리트 미니언의 순간이동. **NavMesh `GetRandomReachablePointInRadius()`** 사용, 반경 600cm 이내 측방/후방 지점 선택. 모든 전투 레벨에 NavMesh Bake 필수. 시전/도착 시 `GCN_Wraith_Teleport [Static]` 호출.
- **중간 보스(슈루드, Shrewd) 패턴 GA**:
  - `GA_Shrewd_ExplosiveArrow`: 발판 위 원거리 폭발 화살. `AnimNotify` 시점에 풀링된 발사체 스폰. 착탄 시 스플래시 피해.
  - `GA_Shrewd_QuickFlurry`: 짧은 예비 딜레이 후 복수 화살 연속 발사. 발사 수/간격은 `UBOBossData.AbilityDamageMap`으로 관리.
  - `GA_Shrewd_MeleeCombo`: 근접 페이즈 2~3단 발톱/몸통 콤보. `AnimNotifyState` 기반 Hitbox 활성화.
  - `GA_Shrewd_Lunge`: 돌진 도약. **Motion Warping**으로 착지 오차 보정.
  - `GA_Shrewd_SeedDrop`: 포효 후 중앙 나무에서 씨앗 포드 액터 10~12개 투하(`BTTask_SeedDrop`). 부화 시작과 동시에 `State.Invulnerable` 태그 부여. 모든 씨앗 포드 파괴 시 자동 해제. 씨앗 포드는 `UBlackoutPoolSubsystem` 풀링 대상.
  - `GA_Shrewd_LoSTeleport`: `BTService_LineOfSightCheck`가 0.5초 주기로 서버 측 라인트레이스 수행, 차단 시 발동. NavMesh에서 플레이어 인접 유효 지점으로 `SetActorLocation()` 이동 후 즉시 `GA_Shrewd_MeleeCombo` 자동 활성화. `GCN_Shrewd_LoSTeleport [Static]` 트리거.
- **메인 보스(타락한 약탈자, Corrupted Ravager) 패턴 GA**:
  - `GA_Ravager_DoubleSwipe`: 넓은 대각선 2연속 할퀴기 엇박자 콤보.
  - `GA_Ravager_TurnBite`: 타겟이 후방/측면 위치 시 빠르게 회전하며 시전하는 카운터 물기.
  - `GA_Ravager_BackwardJump`: 거리 확보용 후방 도약 이동 처리.
  - `GA_Ravager_LungeAttack`: 원거리 강습 도약 후 할퀴기/물기 복합 연계 콤보격. Motion Warping 적용.
  - `GA_Ravager_Shockwave`: 앞발 에너지 차지 후 바닥을 타고 날아가는 가로 장풍격(Projectile). `AnimNotify` 시점 풀링 스폰. `GCN_Ravager_Shockwave_Launch` 지면 파쇄 연출.
  - `GA_Ravager_Howl_Summon`: 하울링 몽타주와 함께 미니언 스폰 데이터를 읽어 동적 스폰.
  - `GA_Ravager_Howl_AoE`: Phase B 광역 에너지 폭발. 제자리 웅크림 차지 후 주변 넓은 반경에 치명 피해. `GCN_Ravager_Howl` 음파 이펙트.
  - `GA_Ravager_Gorenado`: Phase C 궁극기. 다단 히트(Tick) 볼텍스 장판 생성. 플레이어 끌어당김은 `AddForce`/`AddImpulse` 대신 **서버에서 매 Tick마다 `SetActorLocation()`으로 강제 위치 이동** 처리(네트워크 물리 오차 방지). 끌어당김 강도는 볼텍스 중심 거리 반비례 점증.
  - `GA_Ravager_PillarCharge`: 엄폐물(기둥) 파괴 전용 돌진기. 기둥과 충돌 시 §8의 Chaos Destruction 트리거.

## 5. 데미지 판정 및 조건부 자원 보상 (Gameplay Effect)
GE와 ExecCalc(실행 계산기)를 사용해, 피격 처리와 기믹 보상을 처리합니다.

- **데미지/회복 이펙트 (`GameplayEffect`)**:
  - `GE_Damage`: 피격 시 대상의 ASC에 `GameplayCue.Character.Hit` 태그를 전달하여 `GCN_HitImpact [Static]` 혈흔/파편 연출 트리거.
  - `GE_HealOverTime`: 블러드 루트 사용 시 지속 시간 동안 녹색 치유 오라(`GCN_HealLoop [Actor]`) 유지.
  - `GE_RelicHeal`: 유물 사용 시 즉각 체력 회복. `HealingEffectiveness`로 보정.
  - `GE_GulSerumBuff`: 60초 지속. 구르기/전력 질주의 스태미나 소비 비율 50% 감소(승수 Multiplier 적용).
  - `GE_Enrage`: Phase B 진입 시 보스에게 적용되는 스탯 펌핑 GE. 이동속도/공격속도 Attribute 증폭.
- **조건부 보상 처리기 (`ExecCalc_CombatReward`)**:
  - 적 액터 사망 시(On Death), 마지막 타격(Killing Blow) 속성(HitTag)을 평가.
  - **조건 평가**:
    - A캐릭터(어썰트): `Kill.Melee` — 근접 무기 처치
    - B캐릭터(데몰리션): `Kill.MultiTarget.Count≥3` — 단일 스플래시 데미지 이벤트로 3마리 이상 동시 처치
    - C캐릭터(스나이퍼): `Kill.WeakSpot` — 약점(헤드) 치명타 처치
  - **드랍 구성** (GDD §4.1 완전 반영):
    | 조건 만족 캐릭터 | 드랍 아이템 | 스폰 방식 |
    |---|---|---|
    | A/B/C 공통 | **주무기 탄약 박스** | 획득자 현재 주무기 타입에 맞는 `PrimaryReserveAmmo` 충전 |
    | A/B/C 공통 | **보조무기 탄약 박스** | 획득자 `SecondaryReserveAmmo` 충전 |
    | A/B/C 공통 | **소모성 회복약** | **블러드 루트 OR 굴 혈청 중 50:50 무작위 드랍** |
  - 조건 만족 시 가해자(Instigator) 주변에 해당 아이템 액터를 풀링 서브시스템에서 `GetFromPool`하여 서버에서 스폰합니다.

### 5.1 플레이어 다운(Downed) 및 관전(Spectator) 모드 제어
멀티플레이 협동을 위한 다단계 데스 생명 주기를 ASC와 Controller 상태를 통해 구현합니다.
- **다운 상태 진입 (`GE_Downed`)**: 체력이 0 이하로 떨어지면 즉각 파괴(Destroy)하지 않고 캡슐 콜리전 프로필을 변경한 후 `GE_Downed` 이펙트를 부여합니다. 이 이펙트는 이동 및 액션 입력을 봉쇄하는 전역 태그(`State.Downed`)를 씌우며, 매 초 체력을 깎는 타이머 로직(`GE_BleedOut`)을 동반합니다.
- **부활 (`GA_Revive`)**: 생존 동료의 상호작용 완료 시, 대상 폰의 `GE_Downed`와 `GE_BleedOut`을 `RemoveActiveGameplayEffect`로 강제 해제하고 기본 체력값으로 복구합니다. 구출자의 `RelicCharges`를 1 차감합니다.
- **완전 사망 및 관전 전환**: 출혈 타이머 소진 시 서버는 해당 플레이어 폰을 `HiddenInGame = true` 및 물리 불가 상태로 만듭니다. 컨트롤러(`APlayerController`)에서는 `ChangeState(NAME_Spectating)` 함수를 호출하여 엔진 자체 관전 모드로 전환하고, `SetViewTargetWithBlend()`로 카메라를 다른 활성 아군 폰으로 강제 바인딩합니다. 관전 중 **[재시작 요청] UI**가 표시되며, 과반수 투표 시 `ABlackoutBattleGameMode`에서 파티 전원을 해당 구역 화톳불로 귀환 처리하는 **`Server_VoteRestart` RPC**를 호출합니다(§7 참조).

### 5.2 보스 약점 부위 피해 배율 처리 (Hitbox Damage Multiplier)
보스의 특정 뼈대(Bone)에 부착된 히트박스 컴포넌트에서 피격 이벤트 발생 시, 피해 배율은 **`SetByCaller` + `ExecCalc_DamageCalc`** 방식으로 처리합니다.

1. 히트박스 컴포넌트에 부위 태그를 `SetByCaller` 키로 등록. (`Body.WeakSpot` → 1.5배 / `Body.ArmoredLimb` → 0.5배)
2. 피격 시 `GE_Damage`에 해당 태그 키와 배율 값을 동적 주입.
3. `ExecCalc_DamageCalc`에서 태그를 읽어 최종 피해량을 산출.

이 방식은 기존 GAS 데이터 주도 설계와 일관성을 유지하며, 기획자가 `UBOBossData` 에디터에서 배율 수치를 직접 조정할 수 있습니다.

**보스별 Hitbox 부위 맵핑:**

| 보스 | 태그 | 부위 | 배율 | 노출 조건 |
|------|------|------|------|-----------|
| **슈루드 (Shrewd)** | `Body.WeakSpot` | 머리(Head) | **1.5배** | 원거리 페이즈 사격 모션 중 / 씨앗 기믹(`State.Invulnerable`) 중 상시 노출 |
| **타락한 약탈자 (Ravager)** | `Body.WeakSpot` | 등 종양(Back Pustules) | **1.5배** | 어그로 전환 시 등이 플레이어에게 노출되는 순간 |
| **타락한 약탈자 (Ravager)** | `Body.ArmoredLimb` | 앞발 장갑(Armored Legs) | **0.5배** | 상시 활성 (피해 감쇠용) |

## 6. 인공지능 (AI) 기믹 제어 (Behavior Tree)
- **미니언 (Minions)**
  - `Root Hollow (일반)`: 플레이어와 거리를 좁힌 뒤 구르기/박치기 공격 태스크(`BTTask_Charge`)를 수행하여 자세 무너짐(Stagger)을 유발합니다.
  - `Root Wraith (엘리트)`: 활을 이용한 2연발 투사체 발사 후, 플레이어의 시야 바깥 영역(NavMesh 노드 쿼리)으로 즉시 점멸(`BTTask_Teleport`)하여 진영을 교란합니다.
- **중간 보스 (슈루드 - Shrewd)**: 메인 전투 돌입 전 파티 화력 검증 관문. Blackboard Key(`BB_IsOnPlatform`)로 원거리(발판)/근접(지면) 페이즈 교대. `BTService_LineOfSightCheck`가 0.5초 주기로 타겟의 LoS를 체크하며, 차단 시 즉각 `GA_Shrewd_LoSTeleport`를 발동합니다. 씨앗 기믹은 `BTTask_SeedDrop`이 담당하며, 씨앗 투하 시 `State.Invulnerable` 태그 부여 → 씨앗 전량 파괴 완료 시 자동 해제됩니다.
- **메인 보스 (타락한 약탈자 - Corrupted Ravager)**: Blackboard Key로 체력을 추적하여 페이즈 분기.
  - `Phase A` (100~60%): `GA_Ravager_DoubleSwipe`, `GA_Ravager_TurnBite`, `GA_Ravager_BackwardJump` 후 `GA_Ravager_Shockwave` 연계, `GA_Ravager_LungeAttack` 등 기동성 압박. 일반 미니언(Root Hollow) 스폰 `GA_Ravager_Howl_Summon`.
  - `Phase B` (60~30%): `GE_Enrage` 적용으로 스탯 펌핑. `GCN_RedMist [Actor]` 지속 이펙트 활성화. 즉사급 범위기 `GA_Ravager_Howl_AoE` 발동 주기 추가. **일반+엘리트 미니언 혼합 스폰** 분기 실행.
  - `Phase C` (30% 이하): 선후딜 감소(애니메이션 PlayRate 승수 적용). 궁극기 `GA_Ravager_Gorenado` 소용돌이 태스크 활성화. 맵의 기둥 상당수가 `GA_Ravager_PillarCharge`로 이미 파괴된 상태로 회피 난이도 상승.

### 6.1 보스 공통 — 타겟팅(어그로) 시스템 ⭐ (신규)
GDD §6.0을 구현하는 전용 모듈입니다. 중간 보스(Shrewd)와 메인 보스(Ravager) 모두에 동일하게 적용됩니다.

- **`UBlackoutAggroComponent` (ActorComponent)**: `ABlackoutBossCharacter`에 부착. 서버 Authority 전용. 0.25초 주기 타이머로 타겟 평가를 수행하며, 결과를 Behavior Tree의 Blackboard Key `BB_CurrentTarget`에 기록합니다.

- **누적 피해 트래킹**: 컴포넌트 내부에 `TMap<TWeakObjectPtr<APlayerState>, float> DamageAccumulator` 보관. 보스가 `GE_Damage`를 수신할 때 `UAbilitySystemComponent::OnGameplayEffectAppliedDelegateToSelf` 바인딩을 통해 Instigator PlayerState별 누적치를 갱신합니다.

- **타겟 선정 우선순위** (GDD §6.0 1:1 매핑):

  | 우선순위 | 조건 | 판정 로직 |
  |---|---|---|
  | **1순위** | 누적 피해량 최대 | `DamageAccumulator`에서 최대값 플레이어. 단, 2위와의 격차가 임계값(기본 15%) 미만이면 2순위로 넘어감. |
  | **2순위** | 가장 가까운 거리 | `FVector::DistSquared` 기준 최근접 생존 플레이어. |
  | **3순위** | 가장 낮은 현재 체력 | `Health` 어트리뷰트 최저 생존 플레이어. |

- **타겟 전환 쿨다운**: `TargetSwitchCooldown = 5.0f` 서버 변수로 관리. 마지막 전환 시점(`LastSwitchTime`) 기록 후, 쿨다운이 끝나기 전에는 1~3순위 재평가 결과가 바뀌어도 타겟을 유지합니다(핑퐁 방지). 단, 현재 타겟이 **다운/사망**한 경우는 쿨다운 무시하고 즉시 전환합니다.

- **누적 피해 감쇠**: 장기전에서 초반 누적 피해량이 지나치게 굳어지는 것을 막기 위해, `DamageAccumulator` 값은 1초마다 2%씩 선형 감쇠(Decay) 됩니다. (데이터 에셋 `UBOBossData.AggroDecayRate`에서 튜닝 가능)

- **튜닝 파라미터**(`UBOBossData`에 등록):
  - `AggroSwitchCooldown` (기본 5.0초)
  - `AggroDamageThreshold` (기본 0.15 = 15%)
  - `AggroDecayRate` (기본 0.02 /sec)

- **디자인 의도 검증**: A캐릭터가 지속적으로 화력을 뿜으면 1순위에서 자연스럽게 어그로를 유지하고, B/C는 뒤에서 약점/미니언을 안전하게 처리하는 구도가 형성됩니다. A가 다운되면 2순위(근접도) → 다른 전방 플레이어로 자동 인계됩니다.

## 7. 전투 세션 플로우 및 체크포인트 제어 ⭐ (신규)
GDD §2의 게임 플로우와 §8.1의 로비 흐름을 완전히 구현하는 섹션입니다.

### 7.1 로비 흐름 (`ABlackoutLobbyGameMode`)
1. **자동 매치메이킹 완료**: 4명 접속 시 `ABlackoutLobbyGameMode::PostLogin`에서 각 `APlayerController`에게 `Client_OpenClassSelectUI` RPC 호출 → 전체화면 캐릭터 선택 UI 자동 팝업.
2. **캐릭터 선택 리플리케이션**: 플레이어가 병과를 고르면 `Server_SelectClass(FGameplayTag)` RPC → `ABlackoutPlayerState::SelectedClassTag`에 저장 후 리플리케이트. 다른 클라이언트는 `OnRep_SelectedClassTag`로 UI 갱신(누가 무엇을 골랐는지 실시간 표시). **중복 픽은 허용**되므로 서버는 중복 검증을 수행하지 않습니다.
3. **선택 확정 시 로비 장비 지급**: 서버가 `UBOCharacterData`를 참조하여 해당 캐릭터의 무기/어트리뷰트를 플레이어에게 주입. 인게임 조작 가능 상태로 전환.
4. **로비 내 자원 정책** (GDD §8.1 반영):
   - **탄약 무제한**: 로비 레벨에서 `GA_FireWeapon` 실행 시 Cost 체크를 건너뛰는 `LobbyTag.InfiniteAmmo` 분기 사용.
   - **유물/소모품 즉시 재충전**: `ABlackoutPlayerState::OnAttributeChanged(RelicCharges)` 및 소모품 차감 이벤트에서 로비 월드인 경우 `GE_LobbyAutoRefill`을 0.1초 지연 후 재적용.
5. **화톳불 재선택**: 로비 중앙 화톳불 오브젝트(`ABlackoutBonfire`)는 `IBlackoutInteractable` 구현. `E` 상호작용 시 `Server_RequestReopenClassSelect` RPC → 해당 플레이어만 선택 UI 재호출.
6. **Ready Check 및 레벨 전환**: 로비 끝 포털(`ABlackoutPortal`)에 4인 전원이 상호작용하면 `ABlackoutLobbyGameMode::AllPlayersReady` 조건 충족. `ServerTravel("/Maps/Battle_MidBoss")` 실행.

### 7.2 전투 맵 진입 시 자원 초기화 (`ABlackoutBattleGameMode::PostLogin`)
GDD §8.1의 "전투 맵 진입 시 자원 초기화" 규칙을 구현합니다:

| 자원 | 초기화 정책 |
|---|---|
| **탄약 (주/보조 장탄/예비)** | `UBOCharacterData`의 기본 최대치로 **완전 초기화** |
| **유물 충전 횟수** | `MaxRelicCharges(=3)`로 **완전 초기화** |
| **HP/Stamina** | 최대치로 회복 |
| **블러드 루트 / 굴 혈청 소지량** | **최솟값 보정만 수행**. 현재 소지량이 1 미만이면 1로 올리고, 1 이상이면 **유지**. (→ 중간 보스에서 획득한 여분이 메인 보스전으로 이월됨) |

구현은 `ABlackoutPlayerState::ApplyBattleTransitionPolicy(EBattleTransitionType)`로 일원화.

### 7.3 화톳불 체크포인트 및 리스폰 (`ABlackoutBonfire` + `ABlackoutBattleGameMode`)
- **체크포인트 등록**: 전투 맵에는 구역별 화톳불 액터가 배치되어 있으며, `BeginPlay`에서 자신의 구역 태그(`Checkpoint.MidBoss` / `Checkpoint.MainBoss`)를 `ABlackoutBattleGameMode::RegisterCheckpoint`로 등록.
- **마지막 통과 체크포인트 추적**: 플레이어가 화톳불과 상호작용하면 `GameMode::CurrentCheckpointTag`를 해당 구역으로 갱신.
- **휴식 시 효과** (상호작용 `E`):
  - HP/Stamina/탄약/유물 최대치 충전
  - 블러드 루트, 굴 혈청 각각 **소지량이 1 미만이면 1개로 확정 보정** (1 이상은 유지)
  - 해당 구역 내 일반 미니언 리스폰 트리거(`GameMode::RespawnFieldMinions(CheckpointTag)`)
- **파티 전멸 자동 부활**: `GameMode::Tick`에서 생존자 카운트 = 0 감지 시, 모든 플레이어 폰을 `CurrentCheckpointTag`가 가리키는 화톳불 위치에 `Server_RestartAtCheckpoint`로 강제 텔레포트. 이때 모든 `GE_Downed` / `GE_BleedOut`을 제거하고 HP/Stamina/유물을 완전 회복합니다. (로비로 돌아가지 않음)
- **관전 중 과반수 재시작 투표**: §5.1의 `Server_VoteRestart`가 호출되면 동일하게 `Server_RestartAtCheckpoint` 경로를 재사용. 관전 중 플레이어는 다운→완전사망 시퀀스를 건너뛰고 바로 활성 폰으로 복구됩니다.

### 7.4 보스 클리어 및 세션 종료
- **중간 보스 → 메인 보스**: `ABlackoutBattleGameMode::OnMidBossDefeated` 델리게이트 수신 시, 메인 보스 구역 게이트 언락 + 메인 보스방 화톳불을 현재 체크포인트로 갱신. 레벨 트래블은 하지 않고 스트리밍 서브레벨을 로드(`LoadStreamLevel`) 방식으로 처리하여 장비 지속성 보장.
- **메인 보스 처치 (완전 승리)**: `OnMainBossDefeated` 델리게이트 수신 시:
  1. 5초 축하 연출(`GCN_Victory [Static]`) 재생
  2. 각 컨트롤러에 `Client_ShowVictoryScreen` RPC 전송
  3. 추가 5초 대기 후 `ABlackoutBattleGameMode::GameSession->KickPlayer` 또는 `APlayerController::ClientReturnToMainMenuWithTextReason`을 호출하여 **전원 메인 메뉴로 복귀**
  4. 서버 인스턴스는 매치 결과를 로깅 후 `RequestFinishAndExitToMainMenu`로 셧다운 (매치메이킹 큐에 다음 세션 할당 가능 상태로 복귀)

## 8. 엄폐물(기둥) 파괴 시스템 — Chaos Destruction ⭐ (신규)
GDD §6 메인 보스 핵심 기믹인 "돌진 공격에 의한 엄폐물 영구 파괴"를 **Chaos Destruction**으로 구현합니다.

- **액터 구성**: `ABlackoutDestructiblePillar : AActor`
  - 메시: **Geometry Collection** 에셋(`GC_Pillar_Stone`) — 원본 스태틱 메시에서 Fracture Editor로 Voronoi 분할 생성.
  - 콜리전: 파괴 전에는 단순 캡슐/박스 콜리전 사용(물리 비용 절감), 파괴 시에만 Chaos 시뮬레이션 활성화.
  - HP Attribute: 파괴 전까지 `BOPillarHealth` 추적(Ravager의 `GA_Ravager_PillarCharge`만 유효 피해 입힘).
- **파괴 트리거**:
  1. `GA_Ravager_PillarCharge` 히트박스가 기둥과 충돌
  2. 서버에서 `ABlackoutDestructiblePillar::Multicast_Shatter(FVector ImpactPoint, FVector ImpactForce)` RPC 방송
  3. 클라이언트: Chaos Geometry Collection의 `ApplyExternalStrain()` 또는 `CrumbleCluster()` 호출로 분할 시뮬레이션 시작
  4. 잔해는 `ChaosSolver` 물리 시뮬레이션 후 5초 뒤 `SetSimulatePhysics(false)` + LOD 스와프로 정적화
- **네트워크 최적화**: Chaos 파괴는 결정적이지 않으므로 **시뮬레이션 자체는 각 클라이언트에서 독립 실행**하고, 서버는 "파괴됨/파괴안됨" 플래그만 리플리케이트합니다. 신규 접속자 또는 관전자의 카메라 블렌드 시, 이미 파괴된 기둥은 `bIsShattered=true`를 보고 `Instant_Shatter()`로 즉시 잔해 상태로 스폰하여 재현.
- **`ABlackoutGameState::DestroyedPillarIds`** TArray<int32>로 파괴된 기둥 ID 목록을 동기화. Phase C 진입 시 이 배열을 참조하여 회피 난이도 로직(예: 카메라 세이프티 영역 축소)에 반영.
- **성능 가드**: 동시 활성 잔해 클러스터 수가 프로젝트 가이드(50개)를 넘지 않도록, 기둥 파괴 후 8초 경과한 잔해는 서버 Tick에서 강제 숨김 처리(`SetActorHiddenInGame(true)`).

## 9. UI 반응형 바인딩 (HUD) ⭐ (보강)
GDD §8.3의 미니멀리즘 HUD 전체 구성 요소를 UI 위젯 레이어로 명세합니다.

- **`UBlackoutHUDWidget`** (전체 HUD 루트). 각 서브 위젯을 자식으로 포함.
- **플레이어 상태 바인딩** (`Attribute Delegate` 기반, Tick 미사용):
  - `UW_HealthBar` ← `OnHealthChanged`
  - `UW_StaminaBar` ← `OnStaminaChanged` (고갈 임계치 이하 시 점멸 애니메이션 토글)
  - `UW_RelicCounter` ← `OnRelicChargesChanged`
  - `UW_ConsumableSlots` ← `PlayerState::OnRep_BloodRootCount` / `OnRep_GulSerumCount`
  - `UW_AmmoDisplay` ← `OnPrimaryClipAmmoChanged` + `OnPrimaryReserveAmmoChanged` (주/보조 스왑 시 소스 어트리뷰트 전환)
- **파티원 정보 패널 (`UW_PartyRoster`)**: `ABlackoutGameState::PlayerArray` 순회. 각 `ABlackoutPlayerState`의 HP/다운 상태를 `OnRep` 델리게이트로 수신. 다운 시 `REVIVE` 경고 애니메이션과 붉은 해골 아이콘 토글.
- **보스 체력바 (`UW_BossHealthBar`)**: 보스 인카운터 시 `ABlackoutBattleGameMode::OnBossEncounterBegin` 델리게이트로 위젯 활성화. 보스 `ABlackoutBossCharacter`의 `Health`/`MaxHealth` 어트리뷰트에 바인딩. Phase 전환 시 `OnPhaseChanged` 델리게이트 수신하여 단계 표시(Phase A/B/C) 갱신.
- **데미지 플로팅 텍스트**: 로컬 가해자 클라이언트에게만 `Client_ShowDamageNumber` RPC로 전송(§10.4 유지). 치명타 시 색상(노랑/빨강) 및 폰트 크기 트랜지션.
- **크로스헤어 + True Impact Indicator**: `UW_Crosshair`가 Tick에서 카메라 전진 방향 라인트레이스와 무기 `MuzzleSocket` 라인트레이스 결과를 비교하여 실제 착탄 위치 인디케이터를 렌더.
- **월드 플로팅 위젯**: 드랍 아이템/화톳불/포털에 `UWidgetComponent(SpaceType=World)` 부착. `[E] 상호작용` 아이콘을 아이템 바로 위에 표기.

## 10. 최적화 및 리플리케이션
- **Gameplay Cue(GC) 전역 환경 설정**: `AssetManager::InitGlobalData()` 명시적 호출 및 `DefaultGame.ini`에 GC 에셋 검색 경로 등록.
- **UI 델리게이트 반응**: 체력/탄약 HUD는 Tick이 아닌 Attribute Delegate만 사용(§9).
- **월드 플로팅 위젯**: `UWidgetComponent`를 World Space로 부착하여 몰입감 유지.
- **착탄 궤적 연산 (True Impact Indicator)**: 카메라 트레이스 vs 총구 트레이스 오차 보정.
- **플로팅 데미지 인디케이터**: 가해자 전용 Client RPC로 네트워크 부하/시야 방해 최소화.

## 11. 데이터 기반 설계 (Data-Driven)
모든 수치를 데이터 에셋화하여 기획자가 언리얼 에디터 레벨에서 조정할 수 있게 합니다.

- **`UBOCharacterData` (PrimaryDataAsset)**: 플레이어 클래스별 초기 체력, 스태미나 배율, 소모품 기본 지급량, 시작 기본 무기 종류, `GrantedAbilities` 배열.
- **`DT_WeaponStats` (DataTable)**: 무기별 `BaseDamage`, `FireRate`, `MagazineSize`, `SplashRadius`.
- **`UBOMinionData` (PrimaryDataAsset)**: 미니언 스탯(`MaxHealth`, 이동속도), `TMap<GameplayTag, float> AbilityDamageMap`.
- **`UBOBossData` (PrimaryDataAsset)**: 페이즈 체력 컷라인, 미니언 스폰 가중치, 패턴별 데미지 `TMap<GameplayTag, float>`, 어그로 튜닝(`AggroSwitchCooldown`, `AggroDamageThreshold`, `AggroDecayRate`).

## 12. 오브젝트 풀링 (Object Pooling)
- **`UBlackoutPoolSubsystem` (World Subsystem)**: 클래스 타입별 Queue로 풀 관리. `BeginPlay`에서 Pre-warm.

| 타입 | 초기 Pool Size | 산출 기준 |
|------|--------------|----------|
| Root Hollow (일반 미니언) | 20 | 최대 동시 활성화 예상 수 |
| Root Wraith (엘리트 미니언) | 4 | Phase B 이후 최대 동시 활성화 수 |
| 발사체 (화살, 충격파 등) | 액터 수 × 수명(초) × 연사속도(1/초) | 타입별 개별 계산 |
| 탄약 드랍 아이템 | 20 | 최대 동시 바닥 잔존 수 |
| 소모품 드랍 아이템 | 5 | 최대 동시 바닥 잔존 수 |
| Seed Pod (씨앗 포드) | 16 | 슈루드 씨앗 기믹 동시 투하 최대치(12) + 여유 |

- **`IBlackoutPoolableInterface` 의무 구현**
  - `OnSpawnFromPool()`: Hidden 해제, Collision 켜기, Tick 활성화, HP/탄약 리셋
  - `OnReturnToPool()`: Destroy 대신 호출. Hidden 처리, Collision 해제, AI Controller 정지, Tick 비활성화
- **생명 주기 요약**
  - **발사체**: GetFromPool → 충돌 판정 → 임팩트 GC 재생 → OnReturnToPool
  - **드랍 아이템**: 조건부 킬 시 GetFromPool → 바닥 드랍 → 오버랩 획득 또는 수명 만료 → OnReturnToPool
  - **미니언**: GetFromPool(위치/HP 리셋, BT 재실행) → HP 0 시 래그돌 N초 → OnReturnToPool

## 13. 애니메이션 및 로코모션 제어

### 13.1 GAS와 애니메이션 연동 (AnimNotifies & AbilityTasks)
- **PlayMontageAndWait**: GA 내 몽타주 재생 대기 제어.
- **AnimNotifyState (Melee Hitbox)**: 보스 근접 콤보(예: 약탈자 `GA_Ravager_DoubleSwipe`, 슈루드 `GA_Shrewd_MeleeCombo`) 몽타주 구간에 배치, 해당 프레임만 충돌체 활성화.
- **AnimNotify (Projectile Spawn)**: 원거리 투사체(약탈자 Shockwave, 슈루드 Explosive Arrow)를 애니메이션 정확한 타이밍에 풀링 스폰.

### 13.2 모션 워핑 (Motion Warping)
타겟 돌진 공격(`GA_Shrewd_Lunge`, `GA_Ravager_LungeAttack`) 시 애니메이션 고정 이동 거리와 실제 타겟 거리 오차를 해소합니다. 커스텀 AbilityTask와 연동하여 캡슐 트랜지션을 동적으로 증폭/회전, Warp Target에 정확히 착지.

### 13.3 루트 모션 (Root Motion)
무적 구르기(`GA_Dodge`), 플레이어 다운 넉백, 보스 대형 체공 패턴 등은 루트 모션으로 처리. 발 미끄러짐 방지.

### 13.4 애니메이션 블루프린트 (AnimBP) 설계
- **블렌드 스페이스**: 8방향 이동 모션.
- **에임 오프셋**: 카메라 Pitch/Yaw에 따른 상반신 동기화.
- **레이어 블렌딩**: 하반신(이동/구르기) / 상반신(장전/유물/사격 반동) 분리.
- **접지 보정 (Foot IK)**: 1차 구현은 AnimBP `Two Bone IK`. 필요 시 Control Rig 기반 Full Body IK로 마이그레이션 검토(1차 범위 제외).

## 14. 시각 및 음향 효과 (VFX & SFX) 설계

### 14.1 시각 효과 (VFX - Niagara)
- **GameplayCue 시스템 기반 구동**: 서버는 `GameplayTag`만 방송, 각 클라이언트가 로컬에서 Niagara 실행.
- **GameplayCue(GCN) 타입 기준**: 일회성은 `Static`, 지속 루프는 `Actor`.

| GCN 이름 | 타입 | 사유 |
|----------|------|------|
| `GCN_Weapon_Fire` | **Static** | 일회성 총구 화염 |
| `GCN_HitImpact` | **Static** | 일회성 피격 파편 |
| `GCN_Weapon_Reload` | **Static** | 일회성 탄창 탈착음 |
| `GCN_Wraith_Teleport` | **Static** | 순간이동 일회성 연기 |
| `GCN_Shrewd_SeedDrop` | **Static** | 씨앗 투하 일회성 충격 파편/사운드 |
| `GCN_Shrewd_LoSTeleport` | **Static** | LoS 텔레포트 일회성 연기/사운드 |
| `GCN_Ravager_Howl` | **Static** | 일회성 포효 음파 |
| `GCN_Ravager_Shockwave_Launch` | **Static** | 일회성 지면 파쇄 |
| `GCN_Victory` | **Static** | 메인 보스 클리어 일회성 승리 연출 |
| `GCN_HealLoop` | **Actor** | GE 지속 중 루프 치유 오라 |
| `GCN_RedMist` | **Actor** | Phase B 동안 지속 붉은 안개 |

- **주요 효과 분류**: 총기(Muzzle Flash, Tracer, Shell Ejection), 피격(Physical Material별 분기), 보스 기믹(Red Mist, 중력파, 소용돌이).
- **최적화**: 오버드로우 방지를 위한 레이어별 입자 수 제어, `Cull Proxy`.

### 14.2 음향 효과 (SFX - Sound Cue & MetaSounds)
- **공간감 형성 (3D Spatialization)**: Attenuation/Spatialization 일괄 적용.
- **MetaSounds 활용**: 반복 사격/포효에 Pitch/Volume 변주.
- **사운드 우선순위**: 보스 전조음/자기 피격음이 묻히지 않도록 클래스별 볼륨 믹싱 및 우선순위 큐 관리.
- **상태 동기화 사운드**: 스태미나 고갈 헐떡임, 유물 충전음 등.
