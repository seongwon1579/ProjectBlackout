#pragma once

#include "CoreMinimal.h"
#include "BlackoutCharacterBase.h"
#include "BlackoutPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UBOCharacterData;

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

	virtual void PossessedBy(AController* NewController) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Camera")
	TObjectPtr<UCameraComponent> Camera;

	/** 병과별 스탯·어빌리티 데이터. BP 서브클래스(BP_Assault 등)에서 지정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Data")
	TObjectPtr<UBOCharacterData> CharacterData;
};
