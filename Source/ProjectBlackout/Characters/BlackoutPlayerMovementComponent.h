#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BlackoutPlayerMovementComponent.generated.h"

class FSavedMove_BlackoutPlayer;
class FNetworkPredictionData_Client_BlackoutPlayer;

/**
 * 플레이어 전용 이동 컴포넌트입니다.
 * 스프린트 의도를 클라이언트 예측 이동 플래그에 실어 서버와 같은 속도로 시뮬레이션합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UBlackoutPlayerMovementComponent();

	void SetSprintRequested(bool bRequested);
	bool IsSprintRequested() const { return bSprintRequested; }

protected:
	/** 스프린트 중 기본 최대 속도에 곱해질 배수입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Movement", meta = (ClampMin = 1.0))
	float SprintSpeedMultiplier = 1.5f;

	/** 스프린트 입력이 눌린 상태인지 여부 */
	UPROPERTY(Transient)
	bool bSprintRequested = false;

	virtual float GetMaxSpeed() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
};

class FSavedMove_BlackoutPlayer : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
		FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;

private:
	uint8 bSavedSprintRequested : 1;
};

class FNetworkPredictionData_Client_BlackoutPlayer : public FNetworkPredictionData_Client_Character
{
public:
	explicit FNetworkPredictionData_Client_BlackoutPlayer(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
