#pragma once

#include "CoreMinimal.h"
#include "GAS/Abilities/BlackoutGameplayAbility.h"
#include "BlackoutGA_Dodge.generated.h"

class UAnimMontage;
class ABlackoutPlayerCharacter;

/**
 * 방향 입력에 따라 구르기 또는 백스텝을 수행하는 플레이어 회피 GA.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutGA_Dodge : public UBlackoutGameplayAbility
{
	GENERATED_BODY()

public:
	UBlackoutGA_Dodge();
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	/** 현재 액터에서 활성 중인 회피 어빌리티 인스턴스를 찾습니다. */
	static UBlackoutGA_Dodge* GetActiveDodgeAbilityFromActor(const AActor* OwnerActor);

	/** 회피 체인 입력 허용 구간 시작 노티파이에서 호출됩니다. */
	void HandleChainWindowOpened();

	/** 회피 체인 입력 허용 구간 종료 노티파이에서 호출됩니다. */
	void HandleChainWindowClosed();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float StaminaCost = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float DodgeStrength = 900.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float BackstepStrength = 650.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float DodgeDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	float UpwardImpulse = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	TObjectPtr<UAnimMontage> DodgeMontage;
	
	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Ability")
	TObjectPtr<UAnimMontage> DodgeMontage2R;
	*/

private:
	UFUNCTION()
	void OnDodgeFinished();

	bool ConsumeStamina() const;
	bool StartDodgeInternal(ABlackoutPlayerCharacter* PlayerCharacter, const FVector& DodgeDirection, bool bIsBackstep, bool bRestartMontage);
	FVector CalculateDodgeDirection(const FGameplayAbilityActorInfo* ActorInfo, bool& bOutIsBackstep, bool bPreferControlForwardWhenNoInput = false) const;

	FTimerHandle DodgeEndTimerHandle;
	bool bChainWindowOpen = false;
};
