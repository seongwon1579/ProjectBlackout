# Combat — 02. 무기 계층 (Weapon Hierarchy)

> TDD v5 §2.3, §4.1 참조. 무기는 `AActor` 기반. 총기/근접/투사체로 분기. 투사체는 풀링 대상.

```mermaid
classDiagram
    direction TB

    AActor <|-- ABOWeaponBase
    ABOWeaponBase <|-- ABOFirearm
    ABOWeaponBase <|-- ABOMeleeWeapon
    AActor <|-- ABOProjectile

    IBlackoutPoolableInterface <|.. ABOProjectile

    class ABOWeaponBase {
        <<Abstract>>
        #USkeletalMeshComponent* WeaponMesh
        #FGameplayTag WeaponTag
        #FBlackoutWeaponStat CachedStats
        +GetOwningCharacter() ABlackoutCharacterBase*
        +GetWeaponTag() FGameplayTag
        +GetBaseDamage() float
        +AttachToOwner(FName Socket) void
    }

    class ABOFirearm {
        -UNiagaraComponent* MuzzleFlash
        -FName MuzzleSocket
        -TSubclassOf~ABOProjectile~ ProjectileClass
        -bool bUseHitscan
        +Fire(const FVector& Direction) FHitResult
        +SpawnProjectile(const FVector& Direction) ABOProjectile*
        +GetMuzzleTransform() FTransform
    }

    class ABOMeleeWeapon {
        -UBoxComponent* HitBox
        -float SwingRadius
        +PerformSweep(const FVector& Forward) TArray~FHitResult~
        +SetHitBoxActive(bool) void
    }

    class ABOProjectile {
        -UProjectileMovementComponent* Movement
        -USphereComponent* Collision
        -FGameplayEffectSpecHandle DamageSpec
        -float SplashRadius
        +InitFromSpec(const FGameplayEffectSpecHandle&, float Radius) void
        +OnHit(const FHitResult&) void
        +OnSpawnFromPool_Implementation() void
        +OnReturnToPool_Implementation() void
    }

    ABOFirearm ..> ABOProjectile : Spawn
    ABOFirearm ..> UBlackoutPoolSubsystem : 풀링 스폰
    ABOProjectile ..> IBlackoutDamageableInterface : ReceiveDamageFromHitbox
```

## 구현 노트

- **데이터 소싱**: 생성자/BeginPlay에서 `WeaponTag`를 키로 `DT_WeaponStats` 조회 → `CachedStats` 에 복사.
- **Hitscan vs Projectile**: `bUseHitscan = true`면 `LineTraceByChannel`, false면 `ABOProjectile` 을 풀에서 스폰.
- **투사체 풀링**: `ABOProjectile`은 `IBlackoutPoolableInterface` 구현. `UBlackoutPoolSubsystem::SpawnFromPool`로 획득.
  - `OnSpawnFromPool`: Collision/Movement 리셋, `DamageSpec` 주입
  - `OnReturnToPool`: Movement 정지, Collision 비활성화, `DamageSpec` 초기화
- **근접 무기**: `ABOMeleeWeapon::PerformSweep` 결과는 `GA_Melee_Player` 가 수신 → `GE_Damage` 적용.
- **투사체 데미지 전달**: `ABOProjectile`은 `SpecHandle`만 보관하고, `OnHit` 시점에 `IBlackoutDamageableInterface::ReceiveDamageFromHitbox(SpecHandle, BoneName)` 를 호출.
