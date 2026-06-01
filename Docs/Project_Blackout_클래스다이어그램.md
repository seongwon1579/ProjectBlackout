# Project Blackout — 클래스 다이어그램

> Mermaid 문법 기반. TDD v5 §1~§12 참조.

---

## 🏗️ 핵심 게임플레이 구조

### 1. 캐릭터 상속 계층 (Character Hierarchy)

```mermaid
classDiagram
    direction TB

    ACharacter <|-- ABlackoutCharacterBase
    ABlackoutCharacterBase <|-- ABlackoutPlayerCharacter
    ABlackoutCharacterBase <|-- ABlackoutEnemyCharacter
    ABlackoutEnemyCharacter <|-- ABlackoutBossCharacter

    ABlackoutCharacterBase ..|> IAbilitySystemInterface : implements

    class ABlackoutCharacterBase {
        <<Abstract>>
        +GetAbilitySystemComponent() UAbilitySystemComponent*
        #OnDeath() void
        #OnHitReact() void
        #OnStun() void
    }

    class ABlackoutPlayerCharacter {
        -UCameraComponent* Camera
        -USpringArmComponent* SpringArm
        -UBlackoutCombatComponent* CombatComp
        -UBlackoutImpactIndicatorComponent* ImpactIndicatorComp
        +PossessedBy(AController*) void
    }

    class ABlackoutEnemyCharacter {
        -UAbilitySystemComponent* ASC
        +OnSpawnFromPool() void
        +OnReturnToPool() void
        +BeginPlay() void
    }

    class ABlackoutBossCharacter {
        -UBOBossData* BossData
        -UBlackoutAggroComponent* AggroComp
    }

    ABlackoutEnemyCharacter ..|> IBlackoutPoolableInterface : implements
```

---

### 2. GameMode 계층 (Server Only)

```mermaid
classDiagram
    direction TB

    AGameModeBase <|-- ABlackoutGameMode
    ABlackoutGameMode <|-- ABlackoutLobbyGameMode
    ABlackoutGameMode <|-- ABlackoutBattleGameMode

    ABlackoutBattleGameMode --> ABlackoutPlayerState : calls ApplyBattleTransitionPolicy

    class ABlackoutGameMode {
        <<Server Only>>
        +PostLogin(APlayerController*) void
        +Logout(AController*) void
        #CheckPartyWipe() void
        #Server_CommonRPC() void
    }

    class ABlackoutLobbyGameMode {
        +Server_SelectClass(FGameplayTag) void
        +Server_RequestReopenClassSelect() void
        +AllPlayersReady() bool
        -OnReadyCheckComplete() void
        -ServerTravel(FString MapURL) void
    }

    class ABlackoutBattleGameMode {
        -AActor* CurrentCheckpointActor
        +OnMidBossDefeated() void
        +NotifyPlayerFullyDead(ABlackoutPlayerCharacter*) void
        +HandlePartyWipe() void
        +RegisterSurrenderVote(ABlackoutPlayerController*) void
        -EvaluatePartyWipe() void
        -FindNextSpectateTarget(ABlackoutPlayerController*, int32) ABlackoutPlayerCharacter*
        -EvaluateSurrenderVote() void
    }
```

---

### 3. GameState / PlayerState

```mermaid
classDiagram
    direction LR

    class ABlackoutGameState {
        +TArray DestroyedPillarIds
        +float MatchTimer
        +bool bRedMistActive
        +EBossPhase CurrentPhase
        +bool bMidBossDefeated
    }

    class ABlackoutPlayerState {
        -UAbilitySystemComponent* ASC
        -UBlackoutBaseAttributeSet* BaseAttributes
        -UBlackoutPlayerAttributeSet* PlayerAttributes
        -UBlackoutAmmoAttributeSet* AmmoAttributes
        +FGameplayTag SelectedClassTag
        +int32 BloodRootCount
        +int32 GulSerumCount
        +bool bIsReady
        +ApplyBattleTransitionPolicy(EBattleTransitionType) void
        +SetConsumableCounts(int32 BloodRoot, int32 GulSerum) void
        +InitializeConsumablesFromCharacterData(UBOCharacterData*) void
        +OnConsumableCountsChanged(int32 BloodRoot, int32 GulSerum)
        +OnRep_SelectedClassTag() void
        +OnRep_BloodRootCount() void
        +OnRep_GulSerumCount() void
    }

    ABlackoutPlayerState --> UAbilitySystemComponent : owns
    ABlackoutPlayerState --> UBlackoutBaseAttributeSet : has
    ABlackoutPlayerState --> UBlackoutPlayerAttributeSet : has
    ABlackoutPlayerState --> UBlackoutAmmoAttributeSet : has
```

---

### 4. AttributeSet 3종

```mermaid
classDiagram
    direction TB

    UAttributeSet <|-- UBlackoutBaseAttributeSet
    UAttributeSet <|-- UBlackoutPlayerAttributeSet
    UAttributeSet <|-- UBlackoutAmmoAttributeSet

    class UBlackoutBaseAttributeSet {
        +FGameplayAttributeData Health
        +FGameplayAttributeData MaxHealth
        +FGameplayAttributeData MovementSpeed
        +FGameplayAttributeData BaseDamage
        +FGameplayAttributeData DamageReduction
    }

    class UBlackoutPlayerAttributeSet {
        +FGameplayAttributeData Stamina
        +FGameplayAttributeData MaxStamina
        +FGameplayAttributeData CriticalHitChance
        +FGameplayAttributeData CriticalHitMultiplier
        +FGameplayAttributeData HealingEffectiveness
        +FGameplayAttributeData RelicCharges
        +FGameplayAttributeData MaxRelicCharges
    }

    class UBlackoutAmmoAttributeSet {
        +FGameplayAttributeData PrimaryClipAmmo
        +FGameplayAttributeData PrimaryMaxClip
        +FGameplayAttributeData PrimaryReserveAmmo
        +FGameplayAttributeData SecondaryClipAmmo
        +FGameplayAttributeData SecondaryMaxClip
        +FGameplayAttributeData SecondaryReserveAmmo
    }
```

---

### 5. 데이터 에셋 (Data-Driven)

```mermaid
classDiagram
    direction TB

    UPrimaryDataAsset <|-- UBOCharacterData
    UPrimaryDataAsset <|-- UBOMinionData
    UPrimaryDataAsset <|-- UBOBossData
    UPrimaryDataAsset <|-- UBOConsumableData

    class UBOCharacterData {
        +float InitialHealth
        +float InitialStamina
        +TSubclassOf MeleeWeapon
        +TSubclassOf PrimaryWeapon
        +TSubclassOf SecondaryWeapon
        +TArray GrantedAbilities
    }

    class UBOMinionData {
        +float MaxHealth
        +float MovementSpeed
        +TMap AbilityDamageMap
    }

    class UBOBossData {
        +TArray PhaseHealthThresholds
        +TMap AbilityDamageMap
        +float AggroSwitchCooldown
        +float AggroDamageThreshold
        +float AggroDecayRate
        +FMinionSpawnTable SpawnWeights
    }

    class UBOConsumableData {
        <<PrimaryDataAsset>>
        +FGameplayTag ConsumableTag
        +FText DisplayName
        +TSoftObjectPtr~UTexture2D~ Icon
        +int32 InitialCount
        +int32 MaxCount
        +float Cooldown
        +TSubclassOf~UGameplayAbility~ UseAbility
        +TSubclassOf~UGameplayEffect~ GameplayEffect
        +TMap~FGameplayTag, float~ EffectMagnitudes
    }

    class DT_WeaponStats {
        <<DataTable>>
        +float BaseDamage
        +float FireRate
        +int32 MagazineSize
        +float SplashRadius
    }

    note for UBOCharacterData "GrantedAbilities: TArray<TSubclassOf<UGameplayAbility>>"
    note for UBOMinionData "AbilityDamageMap: TMap<FGameplayTag, float>"
    note for UBOBossData "AbilityDamageMap: TMap<FGameplayTag, float>"
    note for UBOConsumableData "소모품별 표시/효과 정의. 현재 소지량은 PlayerState가 복제"
```

---

## ⚔️ 전투 시스템

### 6. GAS 주요 GA/GE 관계

```mermaid
classDiagram
    direction LR

    class GA_FireWeapon {
        +Hitscan / Projectile / Shotgun Pellet 분기
        +Cost: ClipAmmo -1
        +Cue: GCN_Weapon_Fire
    }

    class GA_Reload {
        +ExecCalc_Reload
        +Reserve → Clip 이전
        +Cue: GCN_Weapon_Reload
    }

    class GA_Dodge {
        +I-Frame 무적
        +Root Motion 구르기
        +Tag: State.Invulnerable
        +입력 buffer / late grace
        +timestamp 검증
    }

    class GA_UseRelic {
        +Lock-in 애니메이션
        +GE_RelicHeal 적용
        +RelicCharges -1
        +Tag: State.Locked
    }

    class UBlackoutGA_UseConsumable {
        +소모품 공통 검증
        +PlayerState 수량 차감
        +쿨다운 / 공통 GE 적용
    }

    class UBlackoutGA_UseBloodRoot {
        +ASC 지속 체력 회복
        +Data.Consumable.HealAmount
    }

    class UBlackoutGA_UseGulSerum {
        +ASC 스태미나 소비 배율
        +Data.Consumable.StaminaCostMultiplier
    }

    class GA_Revive {
        +HoldToRevive 진행도
        +GE_Downed 해제
        +구출자 RelicCharges -1
    }

    class GA_Melee_Player {
        +AnimNotifyState Hitbox
        +강철검 / 고철해머 분기
        +콤보 입력 buffer
        +동적 late grace
        +timestamp 검증
    }

    class UBlackoutAbilitySystemComponent {
        +GenericReplicatedEvent InputPressed
        +입력 SequenceId 기록
        +입력 timestamp clamp
    }

    class FBlackoutAbilityInputSyncPayload {
        +SequenceId
        +ClientInputTimeSeconds
        +ClientEstimatedServerTimeSeconds
        +InputTag
    }

    class ExecCalc_DamageCalc {
        +BaseDamage x CritMul
        +x HitZoneMultiplier
        +x 1 - DamageReduction
    }

    class ExecCalc_CombatReward {
        +DropItemClass (BP 기본값)
        +Kill.Melee → 드롭 후보 1개 랜덤
        +Kill.MultiTarget.Count3 → 드롭 후보 1개 랜덤
        +Kill.WeakSpot → 드롭 후보 1개 랜덤
        +SpawnFromPool(ABlackoutDropItem)
    }

    class GE_CombatReward {
        +Instant
        +BP ExecCalc_CombatReward 실행
    }

    class ABOShotgunFirearm {
        +FireShotgun(Direction, DamageSpec)
        +PelletCount / PelletSpreadDegrees
        +TArray~FBlackoutShotgunPelletHit~
    }

    GA_FireWeapon --> ABOShotgunFirearm : 산탄 펠릿 사격
    GA_FireWeapon --> ExecCalc_DamageCalc : 피격 시
    GA_Melee_Player --> ExecCalc_DamageCalc : 피격 시
    GA_Melee_Player ..> UBlackoutAbilitySystemComponent : WaitInputPress
    GA_Dodge ..> UBlackoutAbilitySystemComponent : WaitInputPress
    UBlackoutAbilitySystemComponent ..> FBlackoutAbilityInputSyncPayload : 기록 / 검증
    UBlackoutGA_UseConsumable <|-- UBlackoutGA_UseBloodRoot
    UBlackoutGA_UseConsumable <|-- UBlackoutGA_UseGulSerum
    ABlackoutCharacterBase --> GE_CombatReward : 서버 사망 확정 후
    GE_CombatReward --> ExecCalc_CombatReward : Execution
```

---

### 6.1 플레이어 다운 / 완전 사망

> 상세 설계는 [Foundation/09_Player_Downed_Death.md](Foundation/09_Player_Downed_Death.md)를 기준으로 합니다. 루트 문서는 전체 의존 개요만 유지합니다.

```mermaid
classDiagram
    direction LR

    class ABlackoutCharacterBase {
        +IsDead() bool
        +IsDowned() bool
        #OnDowned() void
        #OnDeath() void
    }

    class ABlackoutPlayerCharacter {
        +TryBeginReviveInteraction(ABlackoutPlayerCharacter*) bool
        +EndReviveInteraction(ABlackoutPlayerCharacter*) void
        +Server_ReviveFromDowned(float) void
        -StartDownedDeathTimer() void
        -HandleDownedDeathTimerExpired() void
    }

    class UBlackoutGA_Revive {
        +ActivateAbility(...) void
        -HandleReviveTick() void
        -FinishRevive() void
        -CancelRevive() void
    }

    class ABlackoutBattleGameMode {
        +NotifyPlayerFullyDead(ABlackoutPlayerCharacter*) void
        -EvaluatePartyWipe() void
        +HandlePartyWipe() void
    }

    class BlackoutGameplayTags {
        +State_Downed
        +State_Reviving
        +State_BeingRevived
        +State_Dead
    }

    ABlackoutCharacterBase <|-- ABlackoutPlayerCharacter
    ABlackoutPlayerCharacter --> UBlackoutGA_Revive : revive target
    ABlackoutPlayerCharacter --> BlackoutGameplayTags : state tags
    ABlackoutPlayerCharacter --> ABlackoutBattleGameMode : full death notification
    ABlackoutBattleGameMode --> ABlackoutBattleGameMode : party wipe evaluation
```

---

### 6.2 플레이어 관전 / 항복 투표

> 상세 설계는 [Foundation/10_Player_Spectator_Surrender.md](Foundation/10_Player_Spectator_Surrender.md)를 기준으로 합니다. 관전 대상에는 완전 사망하지 않은 다운 상태 파티원도 포함합니다.

```mermaid
classDiagram
    direction LR

    class ABlackoutPlayerController {
        +EnterSpectatorMode() void
        +ExitSpectatorMode() void
        +Server_RequestSpectateNext() void
        +Server_RequestSpectatePrevious() void
        +Server_RequestSurrenderVote() void
        +Client_SetSpectateTarget(AActor*) void
    }

    class ABlackoutBattleGameMode {
        +RegisterSurrenderVote(ABlackoutPlayerController*) void
        +CancelSurrenderVote(ABlackoutPlayerController*) void
        -FindInitialSpectateTarget(ABlackoutPlayerController*) ABlackoutPlayerCharacter*
        -FindNextSpectateTarget(ABlackoutPlayerController*, int32) ABlackoutPlayerCharacter*
        -EvaluateSurrenderVote() void
        +HandlePartyWipe() void
    }

    class ABlackoutGameState {
        +int32 SurrenderVoteCount
        +int32 RequiredSurrenderVoteCount
    }

    class ABlackoutPlayerState {
        +bool bRequestedSurrender
    }

    class ABlackoutPlayerCharacter {
        +IsDead() bool
        +IsDowned() bool
    }

    ABlackoutPlayerController --> ABlackoutBattleGameMode : spectate / vote RPC
    ABlackoutBattleGameMode --> ABlackoutPlayerCharacter : spectatable if !IsDead
    ABlackoutBattleGameMode --> ABlackoutGameState : vote count replication
    ABlackoutBattleGameMode --> ABlackoutPlayerState : voter state
```

---

### 7. 오브젝트 풀링

```mermaid
classDiagram
    direction TB

    UWorldSubsystem <|-- UBlackoutPoolSubsystem

    class UBlackoutPoolSubsystem {
        -TMap PoolMap
        +SpawnFromPool(UClass*) AActor*
        +ReturnToPool(AActor*) void
        +WarmUp(UClass*, int32 Count) void
    }

    class IBlackoutPoolableInterface {
        <<Interface>>
        +OnSpawnFromPool() void
        +OnReturnToPool() void
    }

    UBlackoutPoolSubsystem --> IBlackoutPoolableInterface : manages

    note for UBlackoutPoolSubsystem "PoolMap: TMap<UClass*, TArray<AActor*>>"
```

---

### 8. 어그로 시스템

```mermaid
classDiagram
    direction LR

    class UBlackoutAggroComponent {
        <<ActorComponent, Server Only>>
        -TMap DamageAccumulator
        -float TargetSwitchCooldown
        -float LastSwitchTime
        +EvaluateTarget() APlayerState*
        -Priority1_DamageAccumulated() APlayerState*
        -Priority2_ClosestDistance() APlayerState*
        -Priority3_LowestHealth() APlayerState*
        -DecayAccumulator(float DeltaTime) void
    }

    ABlackoutBossCharacter --> UBlackoutAggroComponent : has
    UBlackoutAggroComponent --> UBOBossData : reads tuning

    note for UBlackoutAggroComponent "DamageAccumulator: TMap<TWeakObjectPtr<APlayerState>, float>"
```

---

### 9. 엄폐물 파괴 (Chaos Destruction)

```mermaid
classDiagram
    direction LR

    class ABlackoutDestructiblePillar {
        -UGeometryCollectionComponent* GC
        -float BOPillarHealth
        -bool bIsShattered
        +Multicast_Shatter(FVector ImpactPoint, FVector ImpactForce) void
        +Instant_Shatter() void
    }

    ABlackoutDestructiblePillar --> ABlackoutGameState : updates DestroyedPillarIds
```

---

### 10. 플레이어 전투 컴포넌트

```mermaid
classDiagram
    direction LR

    class UBlackoutCombatComponent {
        <<ActorComponent>>
        -bool bIsAiming
        -EWeaponSlot CurrentSlot
        +GetEquippedFirearm() ABOFirearm*
        +GetMuzzleTransform() FTransform
        +SwapWeapon(EWeaponSlot) void
        +StartAim() void
        +StopAim() void
    }

    class UBlackoutImpactIndicatorComponent {
        <<ActorComponent>>
        -FBlackoutImpactIndicatorUpdateKey LastUpdateKey
        -FBlackoutImpactIndicatorData CachedIndicatorData
        +GetImpactIndicatorData(FBlackoutImpactIndicatorData&) bool
        +GetAimTargetHitResult(FHitResult&, FVector& TraceEnd) bool
        +GetTrueImpactHitResult(FHitResult&, FVector& ImpactPoint, FVector& TraceEnd) bool
        -BuildImpactIndicatorUpdateKey(FBlackoutImpactIndicatorUpdateKey&) bool
        -HasImpactIndicatorUpdateInputChanged(FBlackoutImpactIndicatorUpdateKey) bool
        -RefreshCachedImpactIndicatorData(FBlackoutImpactIndicatorUpdateKey) bool
        -GetHitscanImpactHitResult(FHitResult&, FVector&, FVector&) bool
        -GetProjectileImpactHitResult(FHitResult&, FVector&, FVector&, TArray~FBlackoutTrajectoryPointData~*, float*) bool
        -PerformWeaponTrace(FVector Start, FVector End, AActor* IgnoredActor, FHitResult&) bool
    }

    class UBlackoutPlayerAnimInstance {
        <<AnimInstance>>
        -FBlackoutAimOffsetBlendSettings AimOffsetBlendSettings
        +GetAimOffsetBlendSettings() FBlackoutAimOffsetBlendSettings
    }

    class UBlackoutGA_FireWeapon {
        <<GameplayAbility>>
        -BuildFireDirection() FVector
    }

    class FBlackoutAimOffsetBlendSettings {
        <<Struct>>
        +float MuzzleFullDistance
        +float EyeFullDistance
        +float BlendInterpSpeed
        +FVector2D AimOffsetAngleOffset
    }

    class BlackoutAimOffsetMath {
        <<Namespace>>
        +CalculateEyeBlendAlpha(float ProjectedDistance, FBlackoutAimOffsetBlendSettings) float
        +BlendDirection(FVector MuzzleDirection, FVector EyeTargetDirection, float Alpha) FVector
    }

    class FBlackoutImpactIndicatorUpdateKey {
        <<Struct>>
        +bool bIsAiming
        +FRotator CameraRotation
        +FVector MuzzleLocation
        +TWeakObjectPtr~ABOFirearm~ EquippedFirearm
    }

    class EBlackoutTrajectoryVisualState {
        <<Enum>>
        Normal
        FuseInactive
        Occluded
    }

    class FBlackoutTrajectoryPointData {
        <<Struct>>
        +FVector WorldLocation
        +FVector2D ScreenPosition
        +float DistanceFromMuzzle
        +EBlackoutTrajectoryVisualState VisualState
    }

    class FBlackoutImpactIndicatorData {
        <<Struct>>
        +bool bIsVisible
        +bool bHasBlockingHit
        +bool bTargetMismatch
        +bool bUsesProjectilePrediction
        +bool bIsOccludedFromCamera
        +bool bProjectileImpactFuseInactive
        +FVector WorldLocation
        +FVector2D ScreenPosition
        +TArray~FBlackoutTrajectoryPointData~ TrajectoryPoints
    }

    class ABOFirearm {
        +UsesHitscan() bool
        +GetProjectileClass() TSubclassOf~ABOProjectile~
        +GetProjectileLaunchSpeed() float
        +GetProjectileGravityScale() float
        +GetProjectileCollisionRadius() float
        +GetProjectileImpactFuseArmDistance() float
    }

    ABlackoutPlayerCharacter --> UBlackoutCombatComponent : has
    ABlackoutPlayerCharacter --> UBlackoutImpactIndicatorComponent : has
    ABlackoutPlayerCharacter --> UBlackoutPlayerAnimInstance : animation state
    UBlackoutImpactIndicatorComponent --> UBlackoutCombatComponent : reads combat state
    UBlackoutImpactIndicatorComponent --> ABOFirearm : reads weapon/projectile data
    UBlackoutImpactIndicatorComponent --> FBlackoutAimOffsetBlendSettings : reads aim blend tuning
    UBlackoutImpactIndicatorComponent --> BlackoutAimOffsetMath : blends true hit direction
    UBlackoutGA_FireWeapon --> FBlackoutAimOffsetBlendSettings : reads aim blend tuning
    UBlackoutGA_FireWeapon --> BlackoutAimOffsetMath : blends fire direction
    UBlackoutPlayerAnimInstance o-- FBlackoutAimOffsetBlendSettings : owns
    UBlackoutPlayerAnimInstance --> BlackoutAimOffsetMath : blends aim offset
    UBlackoutImpactIndicatorComponent --> FBlackoutImpactIndicatorUpdateKey : caches update key
    UBlackoutImpactIndicatorComponent --> FBlackoutTrajectoryPointData : fills projectile path
    UBlackoutImpactIndicatorComponent --> FBlackoutImpactIndicatorData : fills
```

---

## 🌐 서버 인프라

### 11. 화톳불 / 포털 (Interactable)

```mermaid
classDiagram
    direction LR

    class ABlackoutBonfire {
        +FGameplayTag CheckpointTag
        +Interact(APlayerController*) void
        -ApplyRestEffect() void
        -ReopenClassSelect() void
    }

    class ABlackoutPortal {
        -TArray ReadyFlags
        +Interact(APlayerController*) void
        +CheckAllReady() bool
    }

    class IBlackoutInteractable {
        <<Interface>>
        +Interact(APlayerController*) void
        +GetInteractionWidget() UUserWidget*
    }

    ABlackoutBonfire ..|> IBlackoutInteractable : implements
    ABlackoutPortal ..|> IBlackoutInteractable : implements
```

---

### 12. 매치메이킹 API 서버 (Nest.js)

```mermaid
classDiagram
    direction TB

    class SessionController {
        <<Controller>>
        +POST /sessions
        +GET /sessions
        +POST /sessions/:id/join
        +DELETE /sessions/:id
    }

    class SessionService {
        +create(playerName) GameSession
        +findAll() GameSession[]
        +findOne(sessionId) GameSession
        +join(sessionId, playerName) GameSession
        +remove(sessionId) boolean
    }

    class SessionExpirationListener {
        +onModuleInit() void
        -handleSessionExpired(sessionId) void
        -cleanupPlayerKeys(sessionId) void
    }

    class EventsGateway {
        <<WebSocket :3001>>
        +handleConnection() void
        +handleDisconnect() void
        +emitToSession(sessionId, event, data) void
    }

    class Redis {
        <<Redis :6379>>
        +session:id → GameSession
        +player:name → sessionId
        +token:token → sessionId
        +server:id → DedicatedServer
    }

    class GameSession {
        +string sessionId
        +string status
        +string[] players
        +number maxPlayers
        +string serverIp
        +number serverPort
    }

    class DedicatedServer {
        +string serverId
        +string ip
        +number port
        +string status
    }

    SessionController --> SessionService : uses
    SessionService --> Redis : read/write
    SessionExpirationListener --> Redis : subscribes expired events
    SessionExpirationListener --> EventsGateway : emitToSession
    EventsGateway --> Redis : room management
```
