#pragma once

#include "CoreMinimal.h"
#include "Animation/BlackoutAnimInstanceBase.h"
#include "BlackoutPlayerAnimInstance.generated.h"

class ABlackoutPlayerCharacter;
class AActor;
class UBlackoutCombatComponent;

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

	FORCEINLINE float GetSafeAimDistanceThreshold() const { return SafeAimDistanceThreshold; }
	FORCEINLINE float GetSafeAimBlendRange() const { return SafeAimBlendRange; }
	FORCEINLINE float GetFallbackAimTargetDistance() const { return FallbackAimTargetDistance; }

protected:
	/** 에임 오프셋 값을 갱신합니다. */
	void UpdateAimOffset(float DeltaSeconds);

	/** 에임 오프셋 값을 비활성 상태로 초기화합니다. */
	void ResetAimOffset();

	/** 원격 플레이어용 복제 에임 오프셋 값을 적용합니다. */
	void ApplyReplicatedAimOffset(float DeltaSeconds);

	/** 로컬 에임 오프셋 값을 서버에 송신합니다. */
	void ReplicateAimOffset(float DeltaSeconds);

	/** 현재 장착 무기 기준 왼손 IK 값을 갱신합니다. */
	void UpdateLeftHandIK(const UBlackoutCombatComponent* CombatComponent);

	/** 카메라 중앙 기준 에임 목표 지점을 갱신합니다. */
	void UpdateAimTarget();

	/** 에임 트레이스 시작 위치와 방향을 가져옵니다. */
	bool GetAimTraceViewPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const;

	/** 플레이어 캐릭터 참조 (캐싱) */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<ABlackoutPlayerCharacter> PlayerCharacter;

	/** 조준 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsAiming;

	/** 현재 장착 무기가 양손 무기인지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bIsTwoHanded;

	/** 현재 장착 무기가 왼손 IK 타겟을 제공하는지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bHasLeftHandIKTarget;

	/** 오른손 기준 Bone Space로 변환된 왼손 IK 위치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	FVector LeftHandIKLocation;

	/** Component Space로 변환된 왼손 IK 회전 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	FRotator LeftHandIKRotation;

	/** 왼손 IK 위치를 변환할 기준 본 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	FName LeftHandIKReferenceBoneName = TEXT("Bone_M_Hand_R");

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

	/** 에임 오프셋 복제 송신 간격 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float AimOffsetReplicationInterval = 0.05f;

	/** 마지막 송신 이후 에임 오프셋 변화량 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float AimOffsetReplicationTolerance = 0.5f;

	/** 카메라 중앙에서 에임 목표를 찾는 최대 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Animation")
	float AimTraceDistance = 10000.f;

	/** 에임 오프셋 안정을 위한 최소 안전 거리 (총구와 타겟 간의 거리, cm 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Animation", meta = (ClampMin = "0.0"))
	float SafeAimDistanceThreshold = 50.f;

	/** 기본 조준과 폴백 조준 간의 채터링을 방지하기 위한 LERP 완충 구간 크기 (cm 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Animation", meta = (ClampMin = "0.0"))
	float SafeAimBlendRange = 50.f;

	/** 총구와 타겟이 너무 가깝거나 관통 시 대체하여 지정할 폴백 타겟 거리 (cm 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Animation", meta = (ClampMin = "0.0"))
	float FallbackAimTargetDistance = 150.f;

	/** 현재 에임 오프셋이 바라볼 월드 위치 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	FVector AimTargetLocation = FVector::ZeroVector;

	/** 현재 카메라 중앙 트레이스에 명중한 대상 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<AActor> AimTargetActor;

	/** 카메라 중앙 트레이스가 유효한 대상을 명중했는지 여부 */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bHasAimTarget = false;

	/** 에임 오프셋 복제 송신 누적 시간 */
	UPROPERTY(Transient)
	float AimOffsetReplicationElapsed = 0.f;

	/** 마지막으로 서버에 송신한 에임 오프셋 값 */
	UPROPERTY(Transient)
	FVector2D LastReplicatedAimOffset = FVector2D::ZeroVector;
};
