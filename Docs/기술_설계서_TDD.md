# 프로젝트 블랙아웃 - 기술 설계서 (TDD) v5 (GDD 정합성 반영)

## 0. 네이밍 규칙 (Naming Convention)
- **접두사 전략**: 기본 접두사는 **`Blackout`** 을 사용합니다.
- **파일명 길이 제한(OS 기준 255자 / 엔진 빌드 캐시 안전치 권장)** 에 걸릴 경우에 한해 **`BO`** 로 축약해 사용합니다. (예: `UBlackoutBossData` → 경로가 길어지는 데이터 에셋은 `UBOBossData`)
- **GA/GE/GCN 등 GAS 오브젝트**는 관례상 축약형(`BO`)보다 **기능 네이밍**을 우선합니다(`GA_Ravager_Gorenado`). 접두사는 C++ 클래스와 UObject 파생 에셋에 적용합니다.

## 1. 프레임워크 및 네트워크 아키텍처
Unreal Engine **5.7.4 바이너리 빌드(버전 고정)** 기반의 **Dedicated Server(데디케이티드 서버) 전용** 구조를 채택합니다. *(Listen Server 미지원)* 프로토타입 단계에서는 고정 IP 직접 접속으로 테스트하며, 정식 서비스 전환 시 AWS 상에 전용 매치메이킹 서버를 별도 구축합니다. 게임 플레이 로직 전반에 **GAS(Gameplay Ability System)** 와 **데이터 기반 설계(Data-Driven Design)** 를 적용하여 모듈화 및 확장성을 극대화합니다.

- **GameMode 계층 구조**: 공통 로직은 부모 클래스에 집중하고, 실제 전투 세션은 단일 전투 맵의 `ABlackoutBattleGameMode`가 총괄합니다.
  - **`ABlackoutGameMode` (부모)**: 서버 전용 베이스 클래스. 공통 기능(파티 전멸 판정, PlayerState 초기화, Server RPC 공통 처리) 담당.
  - **`ABlackoutBattleGameMode` : `ABlackoutGameMode`**: 단일 전투 맵 전용. 시작 쉘터 캐릭터 선택, Ready Check, 구역 게이트 개방, 중간 보스 → 메인 보스 흐름, 파티 전멸 체크포인트 복귀, 승리 시 결과창/메인 메뉴 복귀 담당.
  - **`ABlackoutLobbyGameMode` : `ABlackoutGameMode`**: 레거시/프로토타입 로비 전용 클래스입니다. 현재 설계의 기본 흐름에서는 별도 로비 맵과 전투 맵 이동을 사용하지 않습니다.
- **GameState (`ABlackoutGameState`)**: 매치 타이머, 전역 기믹(파괴된 기둥 상태 배열), 붉은 안개 페이즈 전환 여부, 매치 결과/통계 상태 등을 동기화합니다.
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
  - **플래시 라이트 기능 (조작 및 무기 위임)**: 어두운 맵에서의 가시성 확보를 위해 향상된 입력 `T` 키 매핑을 사용하여 플래시 토글 신호를 현재 장착 중인 무기(`ABOFirearm`)에 포워딩합니다. 실제 `USpotLightComponent` 컴포넌트 장착, 기본 활성화(켜짐) 상태 및 온/오프 상태 변수 복제(Replication) 등 실질적인 플래시 제어 권한은 총기 클래스가 담당하도록 분리 설계되었습니다. 또한, 무기 스왑 시 비장착 무기의 불빛이 노출되는 것을 막기 위해 `ABOWeaponBase`에 `OnEquipStateChanged(bool bEquipped)` 가상 함수 인터페이스를 도입하고 `UBlackoutCombatComponent`에서 장착 상태 변화를 전파하여, 장착된 총기만 플레이어가 켜둔 플래시 상태(`bIsFlashlightOn`)에 따라 불빛을 투사하고 비장착 총기는 자동으로 불빛을 숨기도록 설계되었습니다.
- **`ABlackoutEnemyCharacter`**: `ABlackoutCharacterBase`를 상속받는 AI 적 공통 베이스입니다.
  - 모델 자체에 ASC를 소유(`CreateDefaultSubobject`)하며, `BeginPlay` 시점에 자기 자신의 ASC를 초기화합니다.
  - 미니언과 보스가 공유하는 피격/사망/ASC 초기화 경로를 제공하되, 풀링 생명주기는 하위 `ABlackoutMinionCharacter`로 분리합니다.
- **`ABlackoutMinionCharacter`**: `ABlackoutEnemyCharacter`에서 확장되는 일반/정예 미니언 베이스입니다.
  - 풀링 서브시스템 재사용을 위해 `IBlackoutPoolableInterface`를 구현하며, 큐에서 뽑힐 때(`OnSpawnFromPool`) ASC 상태이상/쿨다운/체력과 미니언 데이터(`UBOMinionData`)를 리셋합니다.
  - `ABORootHollow`, `ABORootWraith`가 이를 상속하여 일반 미니언과 정예 미니언 패턴을 분리합니다.
- **`ABlackoutBossCharacter`**: `ABlackoutEnemyCharacter`에서 확장되어, 다단계 페이즈 트랜지션 로직이 특화된 거대 보스입니다.
  - 페이즈에 따른 패턴 변경 제어를 위해 보스 전용 데이터 에셋(`UBOBossData`)을 참조하여 체력 임계점을 틱마다 지속 감시합니다.
  - 부위별 독립적인 피격 판정(등 약점 1.5배, 단단한 다리 껍질 반감 등)을 제어하기 위해, 기본 캡슐 콜리전을 넘어 특정 뼈대(Bone)에 추가적인 충돌(Hitbox) 컴포넌트를 부착하여 데미지 수신을 분배하는 처리를 가집니다.
  - **어그로 관리**: 보스 AI 컨트롤러가 `UBlackoutAggroEvaluator`(§6.1 참조)를 소유하여 타겟 선정 로직을 전담 처리합니다. Shrewd의 `UBlackoutAggroComponent`는 디버그/보조 경로로 남깁니다.

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
- **예측 실행 전 로컬 유효성 검사**
  - `LocalPredicted` 플레이어 GA는 서버 activation 전에 `CanActivateAbility`에서 클라이언트가 이미 복제받은 ASC 어트리뷰트, PlayerState 소모품 수량, 상태 태그, 로컬 쿨다운으로 실행 가능 여부를 먼저 검사합니다.
  - 소모품/유물처럼 몽타주나 이동속도 변경이 즉시 예측되는 GA는 수량 부족, 유물 충전 부족, 회복할 체력 없음, 로컬 쿨다운 중인 경우 클라이언트에서 activation 자체를 거부합니다. 단, 서버 권위 검증은 `ActivateAbility`에서 동일하게 유지하여 복제 지연과 경쟁 상황을 최종 판정합니다.

- **분리된 주요 GA 목록**:
  - `UBlackoutGA_Dodge`: 방향키 기반 회피 동작. 실행 시 몽타주와 함께 순간적인 I-Frame(무적) 이펙트를 부여.
  - `UBlackoutGA_Sprint`: 스태미나를 지속적으로 깎으며 이동속도 증가 상태 부여.
  - `UBlackoutGA_FireWeapon`: 기본 사격 (라인트레이스 스캔 혹은 Projectile 발사체 스폰). Cost로 `PrimaryClipAmmo` 또는 `SecondaryClipAmmo` 1 소모. 사격 시 `GCN_Weapon_Fire`를 통해 총구 화염 및 반동 사운드 출력.
  - `UBlackoutGA_Reload`: 탄창 재장전. 시전 시 장전 몽타주와 함께 `GCN_Weapon_Reload`를 호출하여 재장전 사운드(탄창 탈착음)를 동기화하고, 완료 시점에서 `PrimaryReserveAmmo` → `PrimaryClipAmmo` 로 이전(ExecCalc로 예비탄 차감 및 장탄 보충 동시 처리).
  - `UBlackoutGA_MeleePlayer`: 플레이어 전용 근접 무기(강철 검 등) 타격 로직. 단축키 입력 시 근접 몽타주 재생 및 전방 공격 판정(Sweep) 생성, 콤보 연계 관리.
  - `UBlackoutGA_Revive`: 쓰러진 아군 구출. 완료 시 **구출을 수행한 플레이어의 `RelicCharges` 어트리뷰트 1회 차감** 로직 포함.
  - `UBlackoutGA_UseConsumable`: 소모품 사용 공통 절차. `UBOConsumableData`를 읽어 PlayerState 소지량 차감, 쿨다운, 공통 GameplayEffect SetByCaller 주입을 담당.
  - `UBlackoutGA_UseBloodRoot`: 블러드 루트 사용. PlayerState의 블러드 루트 소지량 1 차감 후 ASC 지속 체력 회복 타이머를 시작.
  - `UBlackoutGA_UseRelic`: 유물(Dragon Heart) 사용. 사용 애니메이션 재생과 동시에 `GE_RelicHeal` 이펙트로 즉각 체력 회복. 완료 시 `RelicCharges` 1회 차감. 시전 중 이동 입력은 유지하며, `State.UseRelic` 태그로 유물 사용 상태만 구분합니다. 이동 차단용 `State.Locked` 태그는 부여하지 않습니다.
  - `UBlackoutGA_UseGulSerum`: 굴 혈청 사용. PlayerState의 굴 혈청 소지량 1 차감 후 ASC 임시 스태미나 소비 배율을 적용하여 60초간 스태미나 소비 50% 감소.
- **미니언 패턴 GA**:
  - `UBlackoutGA_Hollow_Attack`: 미니언(Root Hollow)의 박치기 등 기본 근접 공격. 타격 판정(Sweep, Overlap)에 맞춰 타겟에게 `GE_Damage` 부여.
  - `GA_Wraith_FireArrow`: 엘리트 미니언(Root Wraith)의 원거리 2연발 화살 투사체 발사. 풀비용(Cost) 없이 쿨다운만 적용.
  - `GA_Wraith_Teleport`: 엘리트 미니언의 순간이동. **NavMesh `GetRandomReachablePointInRadius()`** 사용, 반경 600cm 이내 측방/후방 지점 선택. 모든 전투 레벨에 NavMesh Bake 필수. 시전/도착 시 `GCN_Wraith_Teleport [Static]` 호출.
- **중간 보스(약삭빠름, Shrewd) 패턴 GA**:
  - `UBlackoutGA_Shrewd_FireExplosiveArrow`: 발판 위에서 곡사로 원거리 폭발 화살 발사. `AnimNotify` 시점에 풀링된 발사체 스폰. 착탄 시 스플래시 피해.
  - `UBOGA_Shrewd_FireStraightArrow`: 짧은 예비 딜레이 후 직선 화살을 발사하는 Shrewd 기본 원거리 공격. 발사 수/간격은 Shrewd 데이터와 패턴 데이터에서 관리.
  - `UBlackoutGA_Shrewd_TeleportToPoint` / `UBlackoutGA_Shrewd_TeleportByEQS`: 지정 지점 또는 EQS 결과 지점으로 순간이동합니다. `State.Invulnerable`은 텔레포트 연출 중에만 일시 적용합니다.
  - **씨앗 기믹**: 현재 C++ 구현 범위에는 씨앗 투하 GA/Seed Pod 클래스가 없으므로, 발표 기준에서는 보류 기믹으로 취급합니다. 후속 구현 시 별도 GA와 풀링 액터를 추가합니다.
- **메인 보스(타락한 약탈자, Corrupted Ravager) 패턴 GA**:
  - `UBlackoutGA_Ravager_BasicAttack` / `UBlackoutGA_Ravager_HitboxAttack`: 넓은 할퀴기·물기 등 근접 히트박스 기반 공격의 공통 처리.
  - `UBlackoutGA_Ravager_ChaseAttack` / `UBlackoutGA_Ravager_Charge`: 추격·돌진형 이동 공격. 근접 히트박스가 `ABOBreakablePillarActor`에 적중하면 기둥 파괴도 함께 트리거합니다.
  - `UBlackoutGA_Ravager_Evade`: 거리 확보용 회피 이동 처리.
  - `UBlackoutGA_Ravager_Shockwave`: 앞발 에너지 차지 후 바닥을 타고 날아가는 가로 장풍격(Projectile). `AnimNotify` 시점 풀링 스폰. `GCN_Ravager_Shockwave_Launch` 지면 파쇄 연출.
  - `UBlackoutGA_Ravager_SummonMinion`: 하울링 (혹은 몸을 터는) 몽타주와 함께 미니언 스폰 데이터를 읽어 동적 스폰.
  - `UBlackoutGA_Ravager_EnergyBurst`: Phase B 광역 에너지 폭발. 제자리 웅크림 차지 후 주변 넓은 반경에 치명 피해. `GCN_Ravager_Howl` 음파 이펙트.
  - `UBlackoutGA_Ravager_Gorenado`: Phase C 궁극기. 다단 히트(Tick) 볼텍스 장판 생성. 플레이어 끌어당김은 `AddForce`/`AddImpulse` 대신 **서버에서 매 Tick마다 `SetActorLocation()`으로 강제 위치 이동** 처리(네트워크 물리 오차 방지). 끌어당김 강도는 볼텍스 중심 거리 반비례 점증.

### 4.1 플레이어 콤보 입력 동기화 (v2)
플레이어 근접 공격(`UBlackoutGA_MeleePlayer`)과 연속 구르기(`UBlackoutGA_Dodge`)는 `LocalPredicted` GA로 즉시 로컬 반응을 제공하되, 콤보 섹션 진행·체인 회피 재시작·스태미나/무적/데미지 판정은 모두 서버 권위로 확정합니다. v1에서 사용하던 `Multicast_*Montage` 직접 호출 경로와 `AnimNotifyState` 기반 콤보 윈도우 권위는 **클라/서버 비대칭과 이중 점프 충돌**의 원인이 되어 v2에서 폐기합니다.

#### 4.1.1 권위 모델
| 영역 | 권위 |
|---|---|
| 입력 발생 | 클라이언트 (즉시 예측) |
| 콤보 상태 (CurrentComboIndex, 윈도우/그레이스 시각) | 서버 |
| 몽타주 재생·섹션 점프 | 서버 → `FRepAnimMontageInfo` 로 자동 전파 |
| 히트박스 판정 / 데미지 적용 | 서버 (기존 유지) |
| AnimNotify / NotifyState | 시각 effect·히트박스 타이밍 보조 전용 (권위 X) |

#### 4.1.2 몽타주 동기화
- 몽타주 재생은 GAS 표준 `UAbilityTask_PlayMontageAndWait`로 일원화합니다. 서버 GA는 자기 ASC에서 `PlayMontage`를 호출하고, `UAbilitySystemComponent::RepAnimMontageInfo`가 시뮬레이트 프록시에 자동 복제됩니다.
- 콤보 섹션 점프는 **서버에서만 `Montage_SetNextSectionName(CurrentSection, NextSection)`** 으로 수행합니다. 로컬 예측 클라이언트는 OnRep으로 자연 따라잡고, 별도 `Multicast_JumpMeleeMontageSection`/`Multicast_PlayMeleeMontage`/`Multicast_StopMeleeMontage`는 사용하지 않습니다.
- 시뮬레이트 프록시 렐러번시 누락 시에도 RepAnimMontageInfo의 OnRep이 NextSectionID를 따라잡아 줍니다.

#### 4.1.3 입력 전파 경로
- 입력은 `UAbilityTask_WaitInputPress` + GAS `EAbilityGenericReplicatedEvent::InputPressed` 표준 경로로 일원화합니다. `bReplicateInputDirectly`는 사용하지 않습니다.
- 클라이언트의 `UBlackoutAbilitySystemComponent::HandleAbilityInputPressed`는 활성 GA를 발견하면 `AbilitySpecInputPressed` 호출 직후 **명시적으로 `ServerSetReplicatedEvent(InputPressed, Handle, ScopedPredictionKey, OriginalActivationPredictionKey)`** 를 호출합니다. 이 RPC가 서버 ASC에서 `InvokeReplicatedEvent`를 발화시키고, 서버 GA의 `WaitInputPress::OnPress` 가 자동으로 호출됩니다.
- `FBlackoutAbilityInputSyncPayload`(`SequenceId`, `ClientInputTimeSeconds`, `ClientEstimatedServerTimeSeconds`, `InputTag`, `AbilitySpecHandle`)는 **메타데이터 부가 채널**로 격하합니다. `Server_RecordAbilityInputSyncPayload` 는 timestamp/시퀀스만 기록하며, 표준 RPC와 중복되는 서버 측 `InputPressed` 재발화는 수행하지 않습니다.
- 메타데이터는 서버 grace clamp 계산에만 사용하고, 입력 트리거 자체는 표준 GAS 경로가 담당합니다.

#### 4.1.4 콤보 윈도우 — 서버 World Time 타이머
- 각 콤보 섹션은 데이터로 정의된 `FBlackoutComboSectionDef { SectionName, WindowOpenAtSeconds, WindowCloseAtSeconds, RecoveryEndAtSeconds }` 를 가집니다.
- 서버 GA는 섹션 진입 시 `GetServerWorldTimeSeconds()` 기준으로 윈도우 open/close 타이머를 `SetTimer`하여 자체적으로 윈도우 상태를 관리합니다.
- `AnimNotifyState`(콤보 윈도우 begin/end, 체인 윈도우 open) 는 **시각 effect 트리거 및 히트박스 활성/비활성**에만 사용하고, 콤보 상태 머신에 영향을 주지 않습니다.

#### 4.1.5 버퍼 / 그레이스 / 핑 보정
| 위치 | 길이 (기본) | 역할 |
|---|---|---|
| 클라이언트 ring buffer | 250 ms | 윈도우 열리기 전 도착한 다음 입력 1개 보관 |
| 서버 receive buffer | 150 ms | 윈도우 도래 시 가장 최근 입력 매칭 |
| Late grace | `BaseGrace + RTT*0.5 + Jitter`, 상한 150 ms | 윈도우 종료 후 도착한 입력 허용 |
| Section cancel-into-next | 120~180 ms | 콤보 단계 종료 직후 재진입 허용 시간 |

- 서버 판정 순서: `GA 활성 여부 → SequenceId 단조성 → ClientEstimatedServerTime clamp → 윈도우/그레이스 매칭 → 스태미나·쿨다운·상태 태그`.
- 입력이 어디에도 매칭되지 않으면 **EndAbility를 호출하지 않고** 현재 섹션의 RecoveryEnd까지 재생되도록 두고, 입력 버퍼만 비웁니다. 강제 종료/재시작 체감을 제거하기 위함입니다.

#### 4.1.6 권위 경계
- timestamp·ping 기반 보정은 “입력 수락 여부” 결정에만 사용합니다.
- 히트 판정, I-Frame 부여, 스태미나 소모, 데미지 적용은 서버 현재 상태에서 확정하며, 클라이언트가 보낸 애니메이션 Notify나 적중 결과를 권위 데이터로 사용하지 않습니다.
- 클라이언트의 로컬 예측(섹션 점프 표시, 입력 버퍼 표시)은 서버 RepAnimMontageInfo가 도달하면 자연 reconcile 됩니다. 클라이언트가 자체적으로 `Montage_JumpToSection`을 호출해 권위 결정을 앞서지 않습니다.

## 5. 데미지 판정 및 조건부 자원 보상 (Gameplay Effect)
GE와 ExecCalc(실행 계산기)를 사용해, 피격 처리와 기믹 보상을 처리합니다.

- **데미지/회복 이펙트 (`GameplayEffect`)**:
  - `GE_Damage`: 피격 시 대상의 ASC에 `GameplayCue.Character.Hit` 태그를 전달하여 `GCN_HitImpact [Static]` 혈흔/파편 연출 트리거.
  - 블러드 루트 지속 회복: `UBlackoutGA_UseBloodRoot`가 ASC 지속 체력 회복 타이머를 시작합니다. 별도 GE를 쓸 경우 실제 회복 Modifier가 아니라 Cue/태그 표현 용도로 제한합니다.
  - `GE_RelicHeal`: 유물 사용 시 즉각 체력 회복. `HealingEffectiveness`로 보정.
  - 굴 혈청 버프: `UBlackoutGA_UseGulSerum`이 ASC의 임시 스태미나 소비 배율을 설정해 구르기/전력 질주의 스태미나 소비 비율을 감소시킵니다.
  - `GE_Enrage`: Phase B 진입 시 보스에게 적용되는 스탯 펌핑 GE. 이동속도/공격속도 Attribute 증폭.
- **조건부 보상 처리기 (`ExecCalc_CombatReward`)**:
  - 조건부 킬 보상은 즉시 지급하지 않고, 서버가 월드에 `ABlackoutDropItem` 액터를 스폰한 뒤 플레이어가 `[E]` 상호작용으로 획득할 때 실제 자원을 지급합니다.
  - 보상 판정은 **서버에서 데미지 적용 후 치명 피해가 확정된 직후, `OnDeath()`의 `State.Dead` 부여·풀 반환·AI 정지 같은 사망 사이드 이펙트가 실행되기 전** 적용합니다. `GE_Damage` 안에서 `ExecCalc_DamageCalc`와 같은 실행 순서에 의존하지 않고, `ApplyIncomingDamageSpec`의 서버 권위 치명 피해 분기에서 마지막 타격 Spec을 넘겨 `UBlackoutCombatRewardSettings::CombatRewardEffectClass`에 지정된 `GE_CombatReward`를 적용합니다.
  - `GE_CombatReward`는 Instant GE이며, Execution에는 BP 서브클래스 `ExecCalc_CombatReward`를 지정합니다. 드롭 아이템 액터 클래스(`DropItemClass`)는 캐릭터 BP가 아니라 이 BP ExecCalc의 기본값에서 소유합니다.
  - 보상 조건 태그는 마지막 타격 Spec의 Dynamic Asset Tags에 기록합니다.
    - `GA_MeleePlayer`: 근접 공격 Spec에 `Kill.Melee` 부여.
    - 모든 플레이어 공격 GA/무기 판정: 약점 치명타로 판정된 Spec에 `Kill.WeakSpot` 부여.
    - 모든 플레이어 공격 GA/무기 판정: 단일 데미지 이벤트가 3마리 이상 처치한 경우 해당 이벤트의 Spec에 `Kill.MultiTarget.Count3` 부여.
  - 단일 타겟 처치는 `ApplyIncomingDamageSpec`의 치명 피해 확정 직후 즉시 `GE_CombatReward`를 적용합니다. 산탄·폭발·스플래시처럼 한 입력/투사체가 여러 타겟을 처리하는 경우, GA/무기 판정 소유자가 데미지 배치의 처치 수를 먼저 집계하고 3마리 이상 처치가 확정된 뒤 각 사망 미니언의 보상 Spec에 `Kill.MultiTarget.Count3`를 포함해 `GE_CombatReward`를 후처리 적용합니다.
  - `ExecCalc_CombatReward`는 Source `ABlackoutPlayerState::SelectedClassTag`와 마지막 타격 Spec의 킬 조건 태그를 조합해 보상 여부를 판정합니다.
    - 로비 선택을 거치지 않는 테스트 맵에서는 `ABlackoutPlayerCharacter::CharacterData.ClassTag`를 `SelectedClassTag`의 폴백으로 사용합니다.
    - A캐릭터(어썰트): `Kill.Melee` — 근접 무기 처치
    - B캐릭터(데몰리션): `Kill.MultiTarget.Count3` — 단일 스플래시/광역 데미지 이벤트로 3마리 이상 동시 처치
    - C캐릭터(스나이퍼): `Kill.WeakSpot` — 약점 치명타 처치
  - 조건을 만족하면 일반/정예 미니언(`ABlackoutMinionCharacter`, 보스 제외) 사망 위치에 드롭 후보 중 하나를 무작위로 선택해 1개만 스폰합니다.
    | 드롭 후보 | 확률 | 획득 시 처리 |
    |---|---:|---|
    | **주무기 탄약 박스** | 40% | 획득자 `PrimaryReserveAmmo` 충전 |
    | **보조무기 탄약 박스** | 40% | 획득자 `SecondaryReserveAmmo` 충전 |
    | **소모품 박스** | 20% | 블러드 루트 OR 굴 혈청 중 50:50 무작위 지급 |
  - 드롭 액터는 일반 적 사망 위치 주변에 scatter 후보점을 만든 뒤, `ECC_WorldStatic` 아래 방향 트레이스로 바닥 ImpactPoint를 찾아 `UBlackoutPoolSubsystem::SpawnFromPool`로 서버에서 스폰합니다. 스폰 직후 `ABlackoutDropItem::SnapToGround`가 `PickupMesh` bounds 최하단을 기준으로 최종 Z 위치를 보정합니다.

### 5.1 플레이어 다운(Downed) 및 관전(Spectator) 모드 제어
멀티플레이 협동을 위한 다단계 데스 생명 주기를 ASC와 Controller 상태를 통해 구현합니다.
- **다운 상태 진입 (`GE_Downed`)**: 체력이 0 이하로 떨어지면 즉각 파괴(Destroy)하지 않고 캡슐 콜리전 프로필을 변경한 후 `GE_Downed` 이펙트를 부여합니다. 이 이펙트는 이동 및 액션 입력을 봉쇄하는 전역 태그(`State.Downed`)를 씌우며, 매 초 체력을 깎는 타이머 로직(`GE_BleedOut`)을 동반합니다.
- 다운 상태 진입 시 블러드 루트 같은 지속 체력 회복 타이머는 즉시 취소합니다. 취소된 지속 회복이 소모품에서 시작된 경우 해당 소모품 쿨다운도 서버와 소유 클라이언트에서 초기화합니다. 다운 이후 남은 회복 틱이 플레이어를 자동으로 되살리는 흐름은 허용하지 않습니다.
- **다운 상태 HUD 전환**: 로컬 플레이어가 `State.Downed`에 진입하면 `UBlackoutHUDWidget`은 팀원 상태창(`UW_PartyRoster`)과 보스 체력바(`UW_BossHealthBar`)를 제외한 기본 전투 HUD 레이어를 숨깁니다. 동시에 다운 사망 타이머의 남은 시간을 프로그래스 바로 표시합니다. 타이머 UI는 서버 권위 값(`DownedDeathEndServerTime` 또는 이에 준하는 복제/클라이언트 RPC 값)을 기준으로 클라이언트에서 남은 시간을 계산합니다.
- **부활 (`UBlackoutGA_Revive`)**: 생존 동료의 상호작용 완료 시, 대상 폰의 `GE_Downed`와 `GE_BleedOut`을 `RemoveActiveGameplayEffect`로 강제 해제하고 기본 체력값으로 복구합니다. 구출자의 `RelicCharges`를 1 차감합니다.
- **부활 진행 HUD 전환**: 다운 대상에게 `State.BeingRevived`가 적용되면 서버는 다운 사망 타이머를 일시정지하고, HUD는 사망 타이머 프로그래스 바 대신 `UBlackoutGA_Revive` 진행률 또는 `ReviveEndServerTime`에 기반한 부활 완료까지 남은 시간을 프로그래스 바로 표시합니다. 부활이 취소되면 사망 타이머는 정지 당시 남은 시간부터 재개되고, 부활 성공으로 `State.Downed`와 `State.BeingRevived`가 모두 해제되면 기본 전투 HUD 레이어를 복구합니다.
- **완전 사망 및 관전 전환**: 출혈 타이머 소진 시 서버는 해당 플레이어 폰을 `HiddenInGame = true` 및 물리 불가 상태로 만듭니다. 컨트롤러(`APlayerController`)에서는 `ChangeState(NAME_Spectating)` 함수를 호출하여 엔진 자체 관전 모드로 전환하고, `SetViewTargetWithBlend()`로 카메라를 다른 아군 폰으로 강제 바인딩합니다. 관전 대상은 완전 사망하지 않은 파티원이며, 다운 상태 아군도 포함합니다. 관전자는 다음/이전 대상 변경 입력을 사용할 수 있습니다. 관전 중 **[항복 투표] UI**가 표시되며, 과반수 투표 시 `ABlackoutBattleGameMode`에서 파티 전원을 해당 구역 쉘터로 귀환 처리하는 **`Server_RequestSurrenderVote` RPC**를 호출합니다(§7 참조).

### 5.2 보스 약점 부위 피해 배율 처리 (Hitbox Damage Multiplier)
보스의 특정 뼈대(Bone)에 부착된 히트박스 컴포넌트에서 피격 이벤트 발생 시, 피해 배율은 **`SetByCaller` + `ExecCalc_DamageCalc`** 방식으로 처리합니다.

1. 히트박스 컴포넌트에 부위 태그를 `SetByCaller` 키로 등록. (`Body.WeakSpot` → 1.5배 / `Body.ArmoredLimb` → 0.5배)
2. 피격 시 `GE_Damage`에 해당 태그 키와 배율 값을 동적 주입.
3. `ExecCalc_DamageCalc`에서 태그를 읽어 최종 피해량을 산출.

이 방식은 기존 GAS 데이터 주도 설계와 일관성을 유지하며, 기획자가 `UBOBossData` 에디터에서 배율 수치를 직접 조정할 수 있습니다.

**보스별 Hitbox 부위 맵핑:**

| 보스 | 태그 | 부위 | 배율 | 노출 조건 |
|------|------|------|------|-----------|
| **슈루드 (Shrewd)** | `Body.WeakSpot` | 머리(Head) | **1.5배** | 원거리 페이즈 사격 모션 중 |
| **타락한 약탈자 (Ravager)** | `Body.WeakSpot` | 등 종양(Back Pustules) | **1.5배** | 어그로 전환 시 등이 플레이어에게 노출되는 순간 |
| **타락한 약탈자 (Ravager)** | `Body.ArmoredLimb` | 앞발 장갑(Armored Legs) | **0.5배** | 상시 활성 (피해 감쇠용) |

## 6. 인공지능 (AI) 기믹 제어 (StateTree + 하위 BehaviorTree)
> **AI 프레임워크 원칙**: 미니언은 **순수 StateTree**, 보스는 **StateTree(페이즈 관리) + 하위 BehaviorTree(페이즈별 패턴)** 하이브리드. 상세 자산/노드 설계는 `Docs/AI_Boss/` 참조.

- **미니언 (Minions, 순수 StateTree)**
  - `Root Hollow (일반)`: 플레이어와 거리를 좁힌 뒤 도약/구르기 박치기 Task(`FBSTTask_Charge`)를 수행하여 자세 무너짐(Stagger)을 유발합니다. 보스가 몸을 털 때 붉은 구근(Bulb) 형태로 뿌려진 뒤 즉시 부화.
  - `Root Wraith (엘리트)`: 원거리에서는 2연발 투사체(`FBSTTask_FireTwinArrows`) 발사 후 시야 밖 NavMesh 쿼리 지점으로 즉시 점멸(`FBSTTask_Teleport`). **근접 접근 감지 시 활대를 휘둘러 플레이어를 강하게 밀쳐내는 Push Task(`FBSTTask_BowShove`)** 로 거리를 재확보합니다.
- **중간 보스 (슈루드 - Shrewd, StateTree 페이즈 + 하위 BT)**: 메인 전투 돌입 전 파티 화력 검증 관문. StateTree의 최상위 상태가 원거리(발판) / 근접(지면) **2-Phase Cycling** 을 관리하고, 각 페이즈가 하위 BT를 실행합니다. 타겟팅은 §6.1 **공통 어그로 시스템**을 Ravager와 동일하게 적용합니다.
  - `원거리 페이즈 (Platform)`: 발판 위로 도약 후 `UBlackoutGA_Shrewd_FireExplosiveArrow`(폭발 속성 단발 화살), `UBOGA_Shrewd_FireStraightArrow`(직선 화살) 순환. `UBTService_LineOfSightCheck`가 0.5초 주기로 타겟의 LoS를 체크하여, 차단 시 `UBlackoutGA_Shrewd_TeleportToPoint` 또는 `UBlackoutGA_Shrewd_TeleportByEQS`로 위치를 재선정합니다.
  - `근접 페이즈 (Ground)`: 발판에서 내려와 타겟에게 근접 교전합니다. 근접 콤보/강습 도약은 Shrewd 전용 GA 확장 지점으로 남기며, 현재 구현 범위에서는 원거리 공격과 텔레포트 압박을 우선합니다.
  - **씨앗 기믹**: **기획 보류(GDD §5). 현재 C++ 구현 없음** — 발표 기준의 필수 클리어 기믹에서 제외하고, 후속 범위로 이동합니다.
- **메인 보스 (타락한 약탈자 - Corrupted Ravager, StateTree 3-Phase + 하위 BT)**: StateTree의 `FBSTEval_HealthRatio`가 HP 비율을 지속 퍼블리시하고, `FBSTCond_HealthBelow` 전이로 페이즈 분기.
  - `Phase A` (100~60%): **기동성 압박**. 주요 패턴은 `UBlackoutGA_Ravager_BasicAttack`/`UBlackoutGA_Ravager_HitboxAttack` 기반 근접 콤보입니다.
    - `UBlackoutGA_Ravager_ChaseAttack` / `UBlackoutGA_Ravager_Charge`(도약·추격·돌진형 복합 공격)
    - `UBlackoutGA_Ravager_Evade`(회피 이동) → `UBlackoutGA_Ravager_Shockwave`(앞발 충전 후 바닥을 타고 날아가는 가로 형태 장풍) **연계기**
    - `UBlackoutGA_Ravager_SummonMinion`(Root Hollow/Root Wraith 간헐 스폰).
  - `Phase B` (60~30%): 붉은 안개 포효/전환 연출 후 `GE_Enrage` 적용으로 공격/이동 속도 증가.
    - `GCN_RedMist [Actor]` 지속 이펙트 활성화.
    - 신규 광역기 **`UBlackoutGA_Ravager_EnergyBurst`(제자리에서 웅크리며 붉은 에너지를 충전 후 주변 넓은 반경에 치명적 피해를 주는 에너지 파동)** 주기적 발동.
    - **일반+엘리트(Root Wraith) 미니언 혼합 스폰** 분기 실행 (`UBTTask_SpawnMinionWave` WaveType = `Mixed`).
  - `Phase C` (30% 이하): 체력이 30% 이하로 감소시 광폭화 진입, 선후딜 감소(애니메이션 PlayRate 승수 1.0 → 1.3 적용).
    - Phase A/B 패턴 전체 유지 + 궁극기 `UBlackoutGA_Ravager_Gorenado`(볼텍스 소용돌이로 플레이어를 중앙으로 끌어당김) 신규 추가.
    - 맵의 기둥은 Phase A/B 전투 동안 `UBlackoutGA_Ravager_Charge` 또는 `UBlackoutGA_Ravager_HitboxAttack` 계열 공격의 **히트 부차 효과**로 순차 파괴됩니다. 별도 전용 기둥 돌진 GA를 두지 않고 해당 GA들의 히트 판정이 `ABOBreakablePillarActor`에 적중하면 파괴를 트리거합니다(§8).

### 6.1 보스 공통 — 타겟팅(어그로) 시스템 ⭐ (신규)
GDD §6.0을 구현하는 전용 모듈입니다. 중간 보스(Shrewd)와 메인 보스(Ravager) 모두에 동일하게 적용됩니다.

- **`UBlackoutAggroEvaluator` / StateTree Evaluator (`FBSTEval_ShrewdAggroTarget`, `FBSTEval_WraithAggroTarget`)**: 보스 AI 컨트롤러가 소유하는 `UBlackoutAggroEvaluator`가 공통 타겟 선정을 담당하고, StateTree Evaluator가 이를 페이즈/패턴 그래프에 주입합니다. 서버 Authority 전용이며, 0.25초 주기(`EvaluationInterval`)로 타겟을 평가하여 Controller의 Blackboard Key `BB_CurrentTarget`에 결과를 기록합니다.

- **누적 피해 트래킹**: `UBlackoutAggroEvaluator` 내부에 `TMap<TWeakObjectPtr<APlayerState>, float> DamageAccumulator`를 보관합니다. 보스가 `GE_Damage`를 수신할 때 `RecordDamage(APawn* Source, float Amount)` 경로로 Instigator PlayerState별 누적치를 갱신합니다.

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
GDD §2의 게임 플로우(단일 맵 + 쉘터 집결)를 구현하는 섹션입니다. 별도 로비 GameMode/맵 없이 `ABlackoutBattleGameMode`가 진입·집결·체크포인트를 일원화합니다.

### 7.1 전투 진입 흐름 (`ABlackoutBattleGameMode`, 시작 쉘터)
1. **자동 매치메이킹 완료**: 4명이 데디 서버(단일 전투 맵)에 직접 접속. 별도 로비 맵/레벨 이동 없음. `ABlackoutBattleGameMode::OnPlayerJoined`에서 각 `APlayerController`에게 `Client_OpenClassSelectUI` RPC 호출 → 시작 쉘터에서 캐릭터 선택 UI 팝업.
2. **캐릭터 선택 리플리케이션**: 플레이어가 병과를 고르면 `Server_SelectClass(FGameplayTag)` RPC → `ABlackoutPlayerState::SelectedClassTag`에 저장 후 리플리케이트. 다른 클라이언트는 `OnRep_SelectedClassTag`로 UI 갱신. **중복 픽 허용**, 서버는 중복 검증을 하지 않습니다.
3. **선택 확정 시 장비 지급**: 서버가 `UBOCharacterData`를 참조하여 해당 캐릭터의 무기/어트리뷰트를 주입. 인게임 조작 가능 상태로 전환.
4. **시작 쉘터 자원 정책**: 시작 쉘터(`ABlackoutShelterZone`)는 휴식 효과(탄약/유물/소모품 보정, §7.2)를 적용합니다. 시작 쉘터와 체크포인트 쉘터는 동일한 자원 보정 규칙을 사용합니다.
5. **쉘터 재선택**: 시작 쉘터의 클래스 선택 오브젝트(`ABlackoutClassSelectStone`, `IBlackoutInteractable`)와 `E` 상호작용 시 `Server_RequestReopenClassSelect` RPC → 해당 플레이어만 선택 UI 재호출. (이후 거점 쉘터의 런-중 캐릭터 교체는 동일 경로 재사용.)
6. **Ready Check 및 구역 개방**: 시작 쉘터에서 4인 전원 준비 완료 시 `ABlackoutBattleGameMode::AllPlayersReady` 충족 → `ABlackoutAreaGate`가 **중간 보스 구역 게이트를 언락**합니다(맵 트래블 아님, 단일 맵 내 구역 개방).

### 7.2 시작 쉘터 진입 시 자원 초기화 (`ABlackoutBattleGameMode`)
GDD §2의 자원 초기화 규칙을 구현합니다 (트리거: 시작 쉘터 최초 진입 / 체크포인트 전환):

| 자원 | 초기화 정책 |
|---|---|
| **탄약 (주/보조 장탄/예비)** | `UBOCharacterData`의 기본 최대치로 **완전 초기화** |
| **유물 충전 횟수** | `MaxRelicCharges(=3)`로 **완전 초기화** |
| **HP/Stamina** | 최대치로 회복 |
| **블러드 루트 / 굴 혈청 소지량** | **최솟값 보정만 수행**. 현재 소지량이 1 미만이면 1로 올리고, 1 이상이면 **유지**. (→ 중간 보스에서 획득한 여분이 메인 보스전으로 이월됨) |

구현은 `ABlackoutPlayerState::ApplyBattleTransitionPolicy(EBattleTransitionType)`로 일원화.

### 7.3 쉘터 체크포인트 및 리스폰 (`ABlackoutShelterZone` + `ABlackoutBattleGameMode`)
- **체크포인트 등록**: 전투 맵에는 구역별 쉘터 존이 배치되어 있으며, `BeginPlay`에서 자신의 구역 태그(`Checkpoint.MidBoss` / `Checkpoint.MainBoss`)를 `ABlackoutBattleGameMode::RegisterCheckpoint`로 등록.
- **마지막 통과 체크포인트 추적**: 플레이어가 쉘터 존에 진입하거나 휴식 상호작용을 완료하면 `GameMode::CurrentCheckpointTag`를 해당 구역으로 갱신.
- **휴식 시 효과** (상호작용 `E`):
  - HP/Stamina/탄약/유물 최대치 충전
  - 블러드 루트, 굴 혈청 각각 **소지량이 1 미만이면 1개로 확정 보정** (1 이상은 유지)
  - 해당 구역 내 일반 미니언 리스폰 트리거(`GameMode::RespawnFieldMinions(CheckpointTag)`)
- **파티 전멸 자동 부활**: `GameMode::Tick`에서 생존자 카운트 = 0 감지 시, 모든 플레이어 폰을 `CurrentCheckpointTag`가 가리키는 쉘터 위치에 `Server_RestartAtCheckpoint`로 강제 텔레포트. 이때 모든 `GE_Downed` / `GE_BleedOut`을 제거하고 HP/Stamina/유물을 완전 회복합니다. (로비로 돌아가지 않음)
- **관전 중 과반수 재시작 투표**: §5.1의 `Server_VoteRestart`가 호출되면 동일하게 `Server_RestartAtCheckpoint` 경로를 재사용. 관전 중 플레이어는 다운→완전사망 시퀀스를 건너뛰고 바로 활성 폰으로 복구됩니다.

### 7.4 보스 클리어 및 세션 종료
- **중간 보스 → 메인 보스**: `ABlackoutBattleGameMode::OnMidBossDefeated` 델리게이트 수신 시, **단일 맵 내** 메인 보스 구역 게이트 언락 + 메인 보스방 쉘터를 현재 체크포인트로 갱신. 레벨 트래블/스트리밍 없이 같은 월드에서 구역만 개방하므로 장비/어트리뷰트 지속성이 자연 보장됨.
  - **아레나 캡슐화 (`IArenaResettable`)**: 각 보스 구역은 자기 상태 초기화를 캡슐화한 단위(보스/미니언 풀/파괴물). `HandlePartyWipe`·`HandleCheckpoint`가 `CurrentArena->Reset()`을 호출해 결정적 재시작을 보장. *(이 추상화 덕에 후일 메모리/리셋 결정성 문제가 측정되면 해당 구역을 스트리밍 서브레벨로 전환하는 것이 재작성 없이 가능 — 단일 맵이 최종 구조이되 전환 탈출구는 열어 둠.)*
- **메인 보스 처치 (완전 승리)**: `OnMainBossDefeated` 델리게이트 수신 시:
  1. 5초 축하 연출(`GCN_Victory [Static]`) 재생
  2. 각 컨트롤러에 매치 결과 표시 RPC 전송 → `UBlackoutMatchResultWidget` / `UBlackoutMatchResultWidgetController`가 플레이어별 통계를 표시
  3. 결과창 카운트다운 종료 후 `ABlackoutBattleGameMode::GameSession->KickPlayer` 또는 `APlayerController::ClientReturnToMainMenuWithTextReason`을 호출하여 **전원 메인 메뉴로 복귀**
  4. 서버 인스턴스는 매치 결과를 로깅 후 `RequestFinishAndExitToMainMenu`로 셧다운 (매치메이킹 큐에 다음 세션 할당 가능 상태로 복귀)

## 8. 엄폐물(기둥) 파괴 시스템 — 복제 상태 기반 파괴물 ⭐ (신규)
GDD §6 메인 보스 핵심 기믹인 "돌진/근접 공격에 의한 엄폐물 영구 파괴"를 `ABOBreakablePillarActor`로 구현합니다.

- **액터 구성**: `ABOBreakablePillarActor : AActor, IBlackoutDamageable`
  - 온전한 상태는 `WholeMeshComponent`가 담당하고, 파괴 후 잔해는 여러 `UChildActorComponent` 조각으로 구성합니다.
  - 서버 권위 플래그 `bIsBroken`, `bAreBrokenPiecesHidden`을 복제하여 신규 접속자/관전자도 같은 파괴 상태를 재구성합니다.
  - `ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle&, FName BoneName)`에서 Ravager 기원의 피해 Spec만 유효 파괴 트리거로 인정합니다.
- **파괴 트리거**:
  1. `UBlackoutGA_Ravager_Charge` 또는 `UBlackoutGA_Ravager_HitboxAttack` 계열 히트박스가 기둥과 충돌
  2. 서버에서 `ABOBreakablePillarActor::BreakPillar()` 실행
  3. `WholeMeshComponent`를 숨기고 잔해 조각을 표시/물리 활성화한 뒤 `Multicast_PlayBreakDustEffect()`로 먼지 연출 재생
  4. 일정 시간 후 `HideBrokenPieces()`로 잔해 조각을 숨겨 물리/렌더링 비용을 회수
- **네트워크 최적화**: 파괴 연출의 순간 물리는 클라이언트마다 약간 달라도 허용하되, 게임플레이 판정은 `bIsBroken`과 `ABlackoutGameState::DestroyedPillarIds`를 기준으로 동기화합니다.
- **`ABlackoutGameState::DestroyedPillarIds`** TArray<int32>로 파괴된 기둥 ID 목록을 동기화합니다. Phase C 진입 시 이 배열을 참조하여 회피 난이도 로직(예: 카메라 세이프티 영역 축소)에 반영할 수 있습니다.
- **Fast-Retry 리셋 수용기준 (§7.4 `IArenaResettable` 계약)**: 파티 전멸/체크포인트 복귀 시 `ResetPillar()`가 온전한 메시/콜리전/잔해 조각 초기 Transform/복제 플래그를 결정적으로 복구하고, 소유 아레나의 `IBOArenaResettable::ResetArena()`가 `DestroyedPillarIds`를 비웁니다.
- **성능 가드**: 파괴 후 일정 시간이 지난 잔해 조각은 숨김 처리하고 물리를 비활성화하여 동시 활성 물리 조각 수가 프로젝트 가이드(50개)를 넘지 않도록 합니다.

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
- **다운 상태 UI (`UW_DownedState`)**: 로컬 플레이어의 `State.Downed`, `State.BeingRevived`, `State.Dead`를 구독하여 HUD 모드를 전환합니다. 다운 중에는 기본 전투 HUD를 숨기고 사망 타이머 프로그래스 바를 표시합니다. 부활 진행 중에는 사망 타이머를 일시정지한 뒤 사망 타이머 대신 부활 프로그래스 바를 표시합니다. 부활 성공 시 기본 전투 HUD로 복귀하고, 부활 취소 시 남은 사망 타이머 표시로 돌아가며, 완전 사망 시 관전 HUD로 전환합니다.
- **매치 결과 UI (`UBlackoutMatchResultWidget`)**: 메인 보스 처치 후 `UBlackoutMatchResultWidgetController`가 `ABlackoutGameState`/`ABlackoutPlayerState`의 통계를 수집해 플레이어 컬럼과 통계 테이블을 채우고, 자동 복귀 카운트다운을 표시합니다.
- **데미지 플로팅 텍스트**: 로컬 가해자 클라이언트에게만 `Client_ShowDamageNumber` RPC로 전송(§10.4 유지). 치명타 시 색상(노랑/빨강) 및 폰트 크기 트랜지션.
- **크로스헤어 + True Impact Indicator**: `UW_Crosshair`가 Tick에서 카메라 전진 방향 라인트레이스와 무기 `MuzzleSocket` 라인트레이스 결과를 비교하여 실제 착탄 위치 인디케이터를 렌더.
- **월드 플로팅 위젯**: 드랍 아이템, 클래스 선택 오브젝트(`ABlackoutClassSelectStone`), 구역 게이트(`ABlackoutAreaGate`)에 `UWidgetComponent(SpaceType=World)` 부착. `[E] 상호작용` 아이콘을 대상 바로 위에 표기.

## 10. 최적화 및 리플리케이션
- **Gameplay Cue(GC) 전역 환경 설정**: `AssetManager::InitGlobalData()` 명시적 호출 및 `DefaultGame.ini`에 GC 에셋 검색 경로 등록.
- **UI 델리게이트 반응**: 체력/탄약 HUD는 Tick이 아닌 Attribute Delegate만 사용(§9).
- **월드 플로팅 위젯**: `UWidgetComponent`를 World Space로 부착하여 몰입감 유지.
- **착탄 궤적 연산 (True Impact Indicator)**: 카메라 트레이스 vs 총구 트레이스 오차 보정.
- **플로팅 데미지 인디케이터**: 가해자 전용 Client RPC로 네트워크 부하/시야 방해 최소화.

## 11. 데이터 기반 설계 (Data-Driven)
모든 수치를 데이터 에셋화하여 기획자가 언리얼 에디터 레벨에서 조정할 수 있게 합니다.

- **`UBOCharacterData` (PrimaryDataAsset)**: 플레이어 클래스별 초기 체력, 스태미나 배율, 소모품 기본 지급량, 시작 기본 무기 종류, `GrantedAbilities` 배열.
- **`DT_WeaponStats` (DataTable)**: 무기별 `BaseDamage`, `FireRate`, `MagazineSize`, `SplashRadius`, `CrosshairType`(0~5).
- **`UBOMinionData` (PrimaryDataAsset)**: 미니언 스탯(`MaxHealth`, 이동속도), `TMap<GameplayTag, float> AbilityDamageMap`.
- **`UBOBossData` (PrimaryDataAsset)**: 페이즈 체력 컷라인, 부위별 피해 배율, 패턴별 데미지 `TMap<GameplayTag, float>`, 어그로 튜닝(`AggroSwitchCooldown`, `AggroDamageThreshold`, `AggroDecayRate`).
- **`UBORavagerStatData` / `UBORavagerPatternData`**: Ravager 전용 스탯, 페이즈별 패턴 후보, 미니언 스폰/투사체/히트박스 세부값을 분리 보관합니다.
- **`UUBOShrewdData`**: Shrewd 전용 발사체/텔레포트/원거리 패턴 수치를 보관합니다.

## 12. 오브젝트 풀링 (Object Pooling)
- **`UBlackoutPoolSubsystem` (World Subsystem)**: 클래스 타입별 Queue로 풀 관리. `BeginPlay`에서 Pre-warm.

| 타입 | 초기 Pool Size | 산출 기준 |
|------|--------------|----------|
| Root Hollow (일반 미니언) | 20 | 최대 동시 활성화 예상 수 |
| Root Wraith (엘리트 미니언) | 4 | Phase B 이후 최대 동시 활성화 수 |
| 발사체 (화살, 충격파 등) | 액터 수 × 수명(초) × 연사속도(1/초) | 타입별 개별 계산 |
| 탄약 드랍 아이템 | 20 | 최대 동시 바닥 잔존 수 |
| 소모품 드랍 아이템 | 5 | 최대 동시 바닥 잔존 수 |
| Seed Pod (씨앗 포드) | 보류 | 현재 구현 범위 제외. 후속 씨앗 기믹 확정 시 풀 대상에 추가 |

- **`IBlackoutPoolableInterface` 의무 구현**
  - `OnSpawnFromPool()`: Hidden 해제, Collision 켜기, Tick 활성화, 액터별 런타임 상태 리셋
  - `OnReturnToPool()`: Destroy 대신 호출. Hidden 처리, Collision 해제, Timer/Delegate 정리, 필요 시 AI Controller 정지
- **생명 주기 요약**
  - **발사체**: SpawnFromPool → 충돌 판정 → 임팩트 GC 재생 → OnReturnToPool
  - **드랍 아이템**: 조건부 킬 시 SpawnFromPool → 바닥 트레이스 및 PickupMesh bounds 기준 위치 보정 → `[E]` 상호작용 획득 또는 수명 만료 → OnReturnToPool
  - **미니언**: SpawnFromPool(위치/HP 리셋, AI 재실행) → HP 0 시 래그돌/디졸브 N초 → OnReturnToPool

## 13. 애니메이션 및 로코모션 제어

### 13.1 GAS와 애니메이션 연동 (AnimNotifies & AbilityTasks)
- **PlayMontageAndWait**: GA 내 몽타주 재생 대기 제어.
- **AnimNotifyState (Melee Hitbox)**: 보스 근접 콤보(예: 약탈자 `UBlackoutGA_Ravager_HitboxAttack` 계열) 몽타주 구간에 배치, 해당 프레임만 충돌체 활성화.
- **AnimNotify (Projectile Spawn)**: 원거리 투사체(약탈자 Shockwave, 슈루드 Explosive Arrow)를 애니메이션 정확한 타이밍에 풀링 스폰.
- **플레이어 콤보 입력 윈도우**: 플레이어 근접/구르기 재입력은 클라이언트 Notify RPC가 아니라 **서버 World Time 타이머**(`FBlackoutComboSectionDef`의 `WindowOpenAtSeconds`/`WindowCloseAtSeconds`)로 판정합니다. AnimNotifyState는 히트박스 활성/비활성과 시각 effect 트리거 전용이며, 콤보 상태 머신 권위에는 관여하지 않습니다. 자세한 권위 모델은 §4.1 참조.

### 13.2 모션 워핑 (Motion Warping)
타겟 돌진 공격(`UBlackoutGA_Ravager_ChaseAttack`, `UBlackoutGA_Ravager_Charge`) 시 애니메이션 고정 이동 거리와 실제 타겟 거리 오차를 해소합니다. 커스텀 AbilityTask와 연동하여 캡슐 트랜지션을 동적으로 증폭/회전, Warp Target에 정확히 착지.

### 13.3 루트 모션 (Root Motion)
무적 구르기(`UBlackoutGA_Dodge`), 플레이어 다운 넉백, 보스 대형 체공 패턴 등은 루트 모션으로 처리. 발 미끄러짐 방지.

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
| `BP_GCN_ConsumableUse` | **Static/Burst** | `GameplayCue.Consumable.Use` 태그를 받아 Cue 파라미터의 `EffectCauser` 캐릭터 오른손 소켓에 부착되는 일회성 소모품 사용 연출 |
| `BP_GCN_RelicUse` | **Static/Burst** | `GameplayCue.Relic.Use` 태그를 받아 Cue 파라미터의 `EffectCauser` 캐릭터 오른손 소켓에 부착되는 일회성 유물 사용 연출 |
| `GCN_Wraith_Teleport` | **Static** | 순간이동 일회성 연기 |
| `GCN_Shrewd_Teleport` | **Static** | Shrewd 텔레포트 일회성 연기/사운드 |
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
