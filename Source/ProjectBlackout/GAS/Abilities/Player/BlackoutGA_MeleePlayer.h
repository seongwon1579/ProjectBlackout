#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "BlackoutGA_MeleePlayer.generated.h"

class UAnimMontage;
class UAnimInstance;
class UGameplayEffect;

/**
 * 플레이어 근접 공격 게임플레이 어빌리티 (TDD v5 §4.1)
 * 몽타주 재생, AnimNotify 수신을 통한 스윕 검사, 콤보 입력 윈도우 처리를 담당합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_MeleePlayer : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_MeleePlayer();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

	/** 현재 액터에서 활성 중인 근접 어빌리티 인스턴스를 찾습니다. */
	static UBlackoutGA_MeleePlayer* GetActiveMeleeAbilityFromActor(const AActor* OwnerActor);

	/** 근접 공격창 시작 시 노티파이 스테이트에서 호출됩니다. */
	void HandleMeleeAttackWindowBegin();

	/** 근접 공격창 유지 중 노티파이 스테이트에서 매 프레임 호출됩니다. */
	void HandleMeleeAttackWindowTick();

	/** 근접 공격창 종료 시 노티파이 스테이트에서 호출됩니다. */
	void HandleMeleeAttackWindowEnd();

	/** 콤보 입력 윈도우 시작 노티파이에서 호출됩니다. */
	void HandleComboWindowOpened();

	/** 콤보 입력 윈도우 종료 노티파이에서 호출됩니다. */
	void HandleComboWindowClosed();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UAnimMontage> MeleeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TArray<FName> ComboSectionNames;

	/** 근접 타격 시 적용할 데미지 GameplayEffect 클래스 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UFUNCTION()
	void OnMeleeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	FGameplayEffectSpecHandle BuildDamageSpec() const;
	bool JumpToNextComboSection();
	void ResetComboState();
	UAnimInstance* GetAvatarAnimInstance() const;
	void SnapToControllYaw();
	
	UPROPERTY(Transient)
	TObjectPtr<UAnimInstance> ActiveAnimInstance;

	int32 CurrentComboIndex = INDEX_NONE;
	bool bComboWindowOpen = false;
	bool bComboInputQueued = false;
};
