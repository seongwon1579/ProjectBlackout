#pragma once

#include "CoreMinimal.h"
#include "BlackoutCharacterBase.h"
#include "BlackoutPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UBOCharacterData;
class UBlackoutCombatComponent;
class UGameplayEffect;
class UInputAction;

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
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat|Accessors")
	UBlackoutCombatComponent* GetCombatComponent() const { return CombatComponent; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UBlackoutCombatComponent> CombatComponent;

	/** 병과별 스탯·어빌리티 데이터. BP 서브클래스(BP_Assault 등)에서 지정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Data")
	TObjectPtr<UBOCharacterData> CharacterData;
	
	/** 초기 스탯 설정을 위한 Gameplay Effect (GE_Player_InitStats 등) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	/** CharacterData를 기반으로 초기 어트리뷰트 값 설정 (GE 적용) */
	virtual void InitializeAttributes();
	
	
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
	
#pragma endregion
};
