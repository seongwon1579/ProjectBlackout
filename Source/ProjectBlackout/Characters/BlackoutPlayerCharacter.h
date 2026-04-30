#pragma once

#include "CoreMinimal.h"
#include "BlackoutCharacterBase.h"
#include "GameplayTagContainer.h"
#include "BlackoutPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UBOCharacterData;
class UBlackoutCombatComponent;
class UGameplayEffect;
class UInputAction;
class UAnimMontage;

struct FInputActionValue;

/**
 * 플레이어블 캐릭터 (Assault / Demolition / Sniper 공통 베이스).
 * ASC는 ABlackoutPlayerState가 소유 → PossessedBy에서 InitAbilityActorInfo.
 * 무기/전투 로직(CombatComponent)은 Combat 에픽에서 확장.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutPlayerCharacter : public ABlackoutCharacterBase
{
	GENERATED_BODY()

public:
	ABlackoutPlayerCharacter();
	
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat|Accessors")
	UBlackoutCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
	UFUNCTION(BlueprintPure, Category = "Blackout|Input")
	FVector2D GetCachedMoveInput() const { return CachedMoveInput; }
	
	UFUNCTION(BlueprintPure, Category = "Blackout|Input")
	FVector2D GetPendingDodgeInput() const { return PendingDodgeInput; }

	void SetPendingDodgeInput(const FVector2D& NewInput) { PendingDodgeInput = NewInput; }

	UFUNCTION(Server, Reliable, Category = "Blackout|Input")
	void Server_SetPendingDodgeInput(FVector2D NewInput);

	UFUNCTION(Server, Reliable, Category = "Blackout|Debug")
	void Server_RequestDebugSelfDamage(float DamageAmount);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Blackout|State")
	void Server_ReviveFromDowned(float RevivedHealth);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayDodgeMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayDodgeMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayHitReactMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayHitReactMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayWeaponSwapMontage(FGameplayTag TargetWeaponSlotTag, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayWeaponSwapMontage(FGameplayTag TargetWeaponSlotTag, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayMeleeMontage(UAnimMontage* Montage, FName StartSection = NAME_None, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayMeleeMontage(UAnimMontage* Montage, FName StartSection = NAME_None, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_JumpMeleeMontageSection(UAnimMontage* Montage, FName SectionName);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool JumpMeleeMontageSection(UAnimMontage* Montage, FName SectionName);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_StopMeleeMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool StopMeleeMontage(UAnimMontage* Montage, float BlendOutTime = 0.1f);

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
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	/** 다운 상태 진입 직후 1회 재생할 몽타주입니다. 비어 있으면 태그만 적용합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> DownedEnterMontage;

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayDeathMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayDeathMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayDownedEnterMontage(UAnimMontage* Montage, float PlayRate = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayDownedEnterMontage(UAnimMontage* Montage, float PlayRate = 1.f);
	
	
	
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

	UFUNCTION()
	void HandleDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsHitReactMontagePlaying = false;

	UFUNCTION()
	void HandleHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> EquipPrimaryMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimMontage> EquipSecondaryMontage;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsWeaponSwapMontagePlaying = false;

	UFUNCTION()
	void HandleWeaponSwapMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UAnimMontage* GetWeaponSwapMontage(FGameplayTag TargetWeaponSlotTag) const;
	
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
	float AimCameraInterpSpeed = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Movement")
	float DefaultMaxWalkSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Movement")
	float AimMaxWalkSpeed = 420.f;

	/** 다운 상태에서 기어다닐 때 사용할 이동 속도입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Movement")
	float DownedMaxWalkSpeed = 150.f;
	
	
	
	virtual void Tick(float DeltaSeconds) override;
	void UpdateAimCamera(float DeltaSeconds);
	void UpdateAimMovementMode();
	void CacheAimDefaults();
	void ApplyAimMovementMode(bool bIsAiming);
#pragma endregion
	
	
};
