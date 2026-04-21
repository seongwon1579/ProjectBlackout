#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutAnimInstanceBase.h"
#include "BlackoutPlayerAnimInstance.generated.h"

class ABlackoutPlayerCharacter;

/**
 * 플레이어 캐릭터 전용 애니메이션 인스턴스.
 * 무기 장착 상태, 조준, 스킬 상태 등 플레이어 특화 로직을 담당합니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutPlayerAnimInstance : public UBlackoutAnimInstanceBase
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	/** 플레이어 캐릭터 참조 (캐싱) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<ABlackoutPlayerCharacter> PlayerCharacter;

	/** 조준 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsAiming;

	/** 전력질주 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsSprinting;

	/** 에임 오프셋 Yaw (-180 ~ 180) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float AO_Yaw;

	/** 에임 오프셋 Pitch (-90 ~ 90) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float AO_Pitch;

	/** 에임 오프셋 보간 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float AO_InterpSpeed = 15.f;
};
