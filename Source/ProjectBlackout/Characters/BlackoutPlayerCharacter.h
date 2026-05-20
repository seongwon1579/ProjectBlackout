#pragma once

#include "CoreMinimal.h"
#include "BlackoutCharacterBase.h"
#include "BlackoutPullable.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "BlackoutPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UBOCharacterData;
class UBlackoutCombatComponent;
class UBlackoutImpactIndicatorComponent;
class UGameplayEffect;
class UInputAction;
class UAnimMontage;
class ABlackoutPlayerCharacter;
class AActor;

struct FInputActionValue;

DECLARE_MULTICAST_DELEGATE_TwoParams(FBlackoutReviveInteractionStateChangedNativeSignature, ABlackoutPlayerCharacter*, bool);

USTRUCT(BlueprintType)
struct FBlackoutFireMontageEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	FGameplayTag FireAnimTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> Montage = nullptr;
};

USTRUCT(BlueprintType)
struct FBlackoutReloadMontageEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	FGameplayTag ReloadAnimTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> Montage = nullptr;
};

/**
 * 한 번의 네트워크 호출로 여러 무기 GCN을 재생하기 위한 항목.
 */
USTRUCT(BlueprintType)
struct FBlackoutWeaponGameplayCueEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayTag CueTag;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Cue")
	FGameplayCueParameters CueParameters;
};

/**
 * 플레이어블 캐릭터 (Assault / Demolition / Sniper 공통 베이스).
 * ASC는 ABlackoutPlayerState가 소유 → PossessedBy에서 InitAbilityActorInfo.
 * 무기/전투 로직(CombatComponent)은 Combat 에픽에서 확장.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutPlayerCharacter : public ABlackoutCharacterBase, public IBlackoutPullable
{
	GENERATED_BODY()

public:
	ABlackoutPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void ApplyPull(const FPullData& PullData) override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat|Accessors")
	UBlackoutCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat|Accessors")
	UBlackoutImpactIndicatorComponent* GetImpactIndicatorComponent() const { return ImpactIndicatorComponent; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Data")
	const UBOCharacterData* GetCharacterData() const { return CharacterData; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Input")
	FVector2D GetCachedMoveInput() const { return CachedMoveInput; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Input")
	FVector2D GetPendingDodgeInput() const { return PendingDodgeInput; }

	void SetPendingDodgeInput(const FVector2D& NewInput) { PendingDodgeInput = NewInput; }

	UFUNCTION(Server, Reliable, Category = "Blackout|Debug")
	void Server_RequestDebugSelfDamage(float DamageAmount);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Blackout|State")
	void Server_ReviveFromDowned(float RevivedHealth);

	/** 전멸 후 체크포인트 복귀 시 서버가 사망/다운 상태와 전투 가능 상태를 복구합니다. */
	void RestoreFromPartyWipeRestart();

	// 회피 몽타주 RPC/헬퍼는 TDD §4.1 v2 에서 폐기되었습니다.
	// 재생은 GAS 표준 PlayMontageAndWait + ASC::PlayMontage → FRepAnimMontageInfo 자동 복제.

	/** GA_Dodge 가 회피 진행 상태를 외부에 알리기 위해 호출하는 setter. */
	void SetDodgeMontagePlaying(bool bPlaying) { bIsDodgeMontagePlaying = bPlaying; }

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayHitReactMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayHitReactMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayFireMontage(UAnimMontage* Montage, float PlayRate = 1.f, bool bRestartIfPlaying = false);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayFireMontage(UAnimMontage* Montage, float PlayRate = 1.f, bool bRestartIfPlaying = false);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_StopFireMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool StopFireMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayReloadMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayReloadMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_StopReloadMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool StopReloadMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayWeaponSwapMontage(FGameplayTag TargetWeaponSlotTag, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Unreliable, Category = "Blackout|Cue")
	void Multicast_ExecuteWeaponGameplayCue(FGameplayTag CueTag, FGameplayCueParameters CueParameters,
	                                        bool bSkipLocallyControlled);

	UFUNCTION(NetMulticast, Unreliable, Category = "Blackout|Cue")
	void Multicast_ExecuteWeaponGameplayCueBatch(const TArray<FBlackoutWeaponGameplayCueEntry>& CueEntries,
	                                             bool bSkipLocallyControlled);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayWeaponSwapMontage(FGameplayTag TargetWeaponSlotTag, float PlayRate = 1.f);

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	UAnimMontage* GetWeaponSwapMontageForSlot(FGameplayTag TargetWeaponSlotTag) const;

	// 근접 콤보 몽타주 RPC/헬퍼는 v2 (TDD §4.1) 에서 폐기되었습니다.
	// 몽타주 재생/섹션 점프는 GAS 표준 PlayMontageAndWait + ASC::CurrentMontageJumpToSection 으로 처리하고,
	// 시뮬레이트 프록시는 FRepAnimMontageInfo OnRep 으로 자연 따라잡습니다.
	// 오너 클라이언트는 로컬 예측 몽타주 인스턴스가 있어 서버 승인 후 Client RPC 로 섹션/회전을 보정합니다.
	UFUNCTION(Client, Reliable, Category = "Blackout|Animation")
	void Client_JumpMontageToSection(UAnimMontage* Montage, FName SectionName, bool bApplyControlYaw = false,
	                                 float ControlYawDegrees = 0.f);

	// 루트 모션 회피 체인 재시작은 원격 프록시에서 회전/몽타주 리셋을 같은 틱에 맞춰 적용합니다.
	UFUNCTION(NetMulticast, Unreliable, Category = "Blackout|Animation")
	void Multicast_SyncDodgeChainRestart(UAnimMontage* Montage, FName SectionName, float ServerYawDegrees);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayConsumableMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_StopConsumableMontage(UAnimMontage* Montage, float BlendOutTime = 0.25f);

	UFUNCTION(Client, Reliable, Category = "Blackout|Movement|Ability")
	void Client_BeginAbilityMovementOverride(float SpeedMultiplier, bool bStopMovementImmediately, bool bAddLockedTag);

	UFUNCTION(Client, Reliable, Category = "Blackout|Movement|Ability")
	void Client_EndAbilityMovementOverride();

	UFUNCTION(Server, Unreliable, Category = "Blackout|Animation")
	void Server_SetAimOffset(FVector2D NewAimOffset);

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	FVector2D GetReplicatedAimOffset() const { return ReplicatedAimOffset; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	void CommitPendingWeaponSwap();

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	bool IsDodgeMontagePlaying() const { return bIsDodgeMontagePlaying; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	bool IsWeaponSwapMontagePlaying() const { return bIsWeaponSwapMontagePlaying; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	bool IsHitReactMontagePlaying() const { return bIsHitReactMontagePlaying; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	bool IsReviveMontagePlaying() const { return bIsReviveMontagePlaying; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Interaction")
	bool IsReviveInteractionActive() const { return IsBeingRevived(); }

	UFUNCTION(BlueprintPure, Category = "Blackout|Interaction")
	bool IsReviving() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Interaction")
	bool IsBeingRevived() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Interaction")
	AActor* GetFocusedInteractableActor() const { return FocusedInteractableActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "Blackout|Interaction")
	FVector GetFocusedInteractablePromptWorldLocation() const;

	FBlackoutReviveInteractionStateChangedNativeSignature OnReviveInteractionStateChangedNative;

	bool TryBeginReviveInteraction(ABlackoutPlayerCharacter* Reviver);
	void EndReviveInteraction(ABlackoutPlayerCharacter* Reviver);
	bool TryInteractWithFocusedActor();
	bool HasNearbyReviveTarget() const;

	void SetLocalSprintCameraActive(bool bActive) { bIsLocalSprintCameraActive = bActive; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	UAnimMontage* GetFireMontageForTag(FGameplayTag FireAnimTag) const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	UAnimMontage* GetReloadMontageForTag(FGameplayTag ReloadAnimTag, bool bIsTwoHanded) const;

	void HandleAimStateChanged(bool bNewAiming);


	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Input")
	FVector2D PendingDodgeInput = FVector2D::ZeroVector;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UBlackoutCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UBlackoutImpactIndicatorComponent> ImpactIndicatorComponent;

	/** 원격 클라이언트에서 플레이어 에임 오프셋을 재생하기 위한 복제 값입니다. */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = "Blackout|Animation")
	FVector2D ReplicatedAimOffset = FVector2D::ZeroVector;

	/** 병과별 스탯·어빌리티 데이터. BP 서브클래스(BP_Assault 등)에서 지정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Data")
	TObjectPtr<UBOCharacterData> CharacterData;

	/** 초기 스탯 설정을 위한 Gameplay Effect (GE_Player_InitStats 등) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	/** 디버그 자가 피격 테스트에 사용할 Gameplay Effect. GE_Damage를 연결합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug")
	TSubclassOf<UGameplayEffect> DebugSelfDamageEffect;

	/** CharacterData를 기반으로 초기 어트리뷰트 값 설정 (GE 적용) */
	virtual void InitializeAttributes();

	/** 피격 시 플레이어 전용 히트 리액션 몽타주를 재생합니다. */
	virtual void OnHitReact() override;
	virtual void OnDowned() override;
	virtual bool CanEnterDownedState() const override;
	virtual void OnDeath() override;
	virtual void HandleDownedStateChanged(bool bWasDowned, bool bIsDowned) override;
	
	/** 완전 사망 시 이 목록에서 유효한 몽타주 하나를 골라 재생합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TArray<TObjectPtr<UAnimMontage>> DeathMontageVariants;

	/** 다운 상태 진입 직후 1회 재생할 몽타주입니다. 비어 있으면 태그만 적용합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> DownedEnterMontage;

	/** 다운 상태에서 부활 성공 시 기상 연출로 재생할 몽타주입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> ReviveMontage;

	/** 다운 상태에서 이 시간이 지나면 완전 사망으로 전환됩니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|State", meta = (ClampMin = "1.0"))
	float DownedDeathDuration = 30.0f;

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayDeathMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayDeathMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayDownedEnterMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayDownedEnterMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayReviveMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayReviveMontage(UAnimMontage* Montage, float PlayRate = 1.f);

public:
	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayRevivePerformMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayRevivePerformMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_StopRevivePerformMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool StopRevivePerformMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

protected:
	void ApplyDownedStateLocally();
	void ClearDownedStateLocally();
	void RestoreWeaponVisibilityAfterRevive();
	void ScheduleWeaponVisibilityRestoreAfterRevive();
	void HandleReviveMontageEnded(UAnimMontage* Montage, bool bInterrupted);
	void SetRevivingStateActive(bool bNewReviving);
	void SetBeingRevivedStateActive(bool bNewBeingRevived);
	void ApplyReplicatedReviveInteractionStateTag();
	void StartDownedDeathTimer();
	void ClearDownedDeathTimer();
	void HandleDownedDeathTimerExpired();
	void NotifyBattleGameModePlayerFullyDead();
	UAnimMontage* SelectDeathMontage() const;
	
	
	
	// 플레이어 캐릭터 인풋 매핑 세팅 //
#pragma region InputSetup

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> MouseLookAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Input")
	void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Input")
	void DoLook(float Yaw, float Pitch);


#pragma endregion

#pragma region Movement

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Movement")
	float ForwardTurnInterpSpeed = 10.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Input")
	FVector2D CachedMoveInput = FVector2D::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsDodgeMontagePlaying = false;

	UPROPERTY(Transient)
	float CachedAbilityOverrideMaxWalkSpeed = 0.0f;

	UPROPERTY(Transient)
	bool bAppliedLocalAbilityLockedTag = false;

	// UFUNCTION()
	// HandleDodgeMontageEnded 는 TDD §4.1 v2 에서 폐기 (PlayDodgeMontage 와 함께 제거).
	// GA_Dodge::EndAbility 가 SetDodgeMontagePlaying(false) 로 플래그를 직접 정리합니다.

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsHitReactMontagePlaying = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsReviveMontagePlaying = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsDeathMontagePlaying = false;

	/** 서버의 State.BeingRevived 태그를 클라이언트 로컬 ASC 태그로 옮기기 위한 복제 브리지입니다. */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_ReviveInteractionActive, BlueprintReadOnly, Category = "Blackout|Interaction")
	bool bIsReviveInteractionActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Interaction")
	float InteractionSearchRadius = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Interaction", meta = (ClampMin = 0.0))
	float InteractionScanInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Interaction")
	float InteractionPromptHeightOffset = 24.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Interaction")
	TWeakObjectPtr<AActor> FocusedInteractableActor;

	UPROPERTY(Transient)
	float InteractionScanElapsed = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Camera")
	bool bIsLocalSprintCameraActive = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<ABlackoutPlayerCharacter> ActiveReviver;

	FTimerHandle ReviveWeaponRestoreTimerHandle;
	FTimerHandle DownedDeathTimerHandle;

	UFUNCTION()
	void HandleHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void HandleDeathMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnRep_ReviveInteractionActive();

	void BroadcastReviveInteractionStateChanged();
	void UpdateFocusedInteractable(float DeltaSeconds);
	void RefreshFocusedInteractableActor();
	bool IsValidFocusedInteractable(AActor* CandidateActor) const;

	UFUNCTION(Server, Reliable, Category = "Blackout|Interaction")
	void Server_InteractWithActor(AActor* TargetActor);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TArray<FBlackoutFireMontageEntry> FireMontageEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TArray<FBlackoutReloadMontageEntry> ReloadMontageEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> ReloadFallbackMontage1R;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> ReloadFallbackMontage2R;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> EquipPrimaryMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> EquipSecondaryMontage;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsWeaponSwapMontagePlaying = false;

	UFUNCTION()
	void HandleWeaponSwapMontageEnded(UAnimMontage* Montage, bool bInterrupted);

#pragma endregion

#pragma region Aim

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	float DefaultArmLength = 350.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	float AimArmLength = 230.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	FVector DefaultSocketOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	FVector AimSocketOffset = FVector(0.f, 55.f, 12.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	float DefaultFOV = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	float AimFOV = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	float SprintFOV = 96.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	float DodgeFOV = 94.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Camera")
	float AimCameraInterpSpeed = 12.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Movement")
	float DefaultMaxWalkSpeed;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Movement")
	float AimMaxWalkSpeed;

	/** 다운 상태에서 기어다닐 때 사용할 이동 속도입니다. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Movement")
	float DownedMaxWalkSpeed;


	virtual void Tick(float DeltaSeconds) override;
	void UpdateAimCamera(float DeltaSeconds);
	float ResolveTargetCameraFOV(bool bIsAiming) const;
	void UpdateAimMovementMode();
	void CacheAimDefaults();
	void ApplyAimMovementMode(bool bIsAiming);
#pragma endregion
};
