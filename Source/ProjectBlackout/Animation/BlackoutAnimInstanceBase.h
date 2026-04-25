#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlackoutAnimInstanceBase.generated.h"

class ABlackoutCharacterBase;
class UCharacterMovementComponent;

/**
 * 프로젝트 블랙아웃의 모든 캐릭터(플레이어/몬스터) 애니메이션 블루프린트의 베이스 클래스.
 * 공통적인 이동 데이터 및 GAS 상태 태그 연동을 담당합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** 소유하고 있는 캐릭터 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<ABlackoutCharacterBase> OwnerCharacter;

	/** 이동 컴포넌트 참조 */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UCharacterMovementComponent> OwnerMovementComponent;

	/** 지면 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float GroundSpeed;

	/** 공중 체류 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsFalling;

	/** 가속도 존재 여부 (입력 여부) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsAccelerating;

	/** 현재 속도 벡터 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	FVector Velocity;

	/** 이동 방향 (-180 ~ 180도) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float MovementDirection;

	/** 다운 상태 여부 (State.Downed 태그 기반) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsDowned;

	/** 행동 제약 상태 여부 (State.Locked 태그 기반) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsLocked;
};
