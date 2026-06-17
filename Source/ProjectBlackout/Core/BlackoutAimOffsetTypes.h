// ─── 구현 내역 ───────────────────────
//  - 김민영: 총구↔눈 위치 기준 에임 오프셋 전환 공용 설정 구조체와 거리 기반 블렌드 알파/방향 보간 수학 함수
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutAimOffsetTypes.generated.h"

/**
 * 총구 기준 조준과 눈 위치 기준 조준 사이를 전환하기 위한 공용 설정입니다.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutAimOffsetBlendSettings
{
	GENERATED_BODY()

	/** 이 거리 이상에서는 총구 기준 조준을 완전히 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Animation", meta = (ClampMin = "0.0"))
	float MuzzleFullDistance = 500.f;

	/** 이 거리 이하에서는 눈 위치 기준 조준을 완전히 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Animation", meta = (ClampMin = "0.0"))
	float EyeFullDistance = 200.f;

	/** 총구 기준과 눈 위치 기준 사이의 전환 알파 보간 속도입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Animation", meta = (ClampMin = "0.0"))
	float BlendInterpSpeed = 12.f;

	/** 눈 위치 기준으로 전환될수록 에임 오프셋에 더해지는 각도 보정값입니다. X=Yaw, Y=Pitch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|Animation")
	FVector2D AimOffsetAngleOffset = FVector2D::ZeroVector;
};

namespace BlackoutAimOffsetMath
{
	/** 투영 거리를 기반으로 눈 위치 기준 조준으로 전환되는 목표 알파를 계산합니다. */
	FORCEINLINE float CalculateEyeBlendAlpha(float ProjectedDistance, const FBlackoutAimOffsetBlendSettings& Settings)
	{
		const float EyeDistance = FMath::Min(Settings.EyeFullDistance, Settings.MuzzleFullDistance);
		const float MuzzleDistance = FMath::Max(Settings.EyeFullDistance, Settings.MuzzleFullDistance);
		const float BlendDistanceRange = MuzzleDistance - EyeDistance;

		float Alpha = 0.f;
		if (ProjectedDistance <= EyeDistance)
		{
			Alpha = 1.f;
		}
		else if (ProjectedDistance >= MuzzleDistance)
		{
			Alpha = 0.f;
		}
		else if (BlendDistanceRange > KINDA_SMALL_NUMBER)
		{
			Alpha = 1.f - ((ProjectedDistance - EyeDistance) / BlendDistanceRange);
		}

		Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
		return Alpha * Alpha * (3.f - 2.f * Alpha);
	}

	/** 현재 전환 알파가 목표 알파를 부드럽게 따라가도록 보간합니다. */
	FORCEINLINE float InterpEyeBlendAlpha(float CurrentAlpha, float TargetAlpha, float DeltaSeconds, const FBlackoutAimOffsetBlendSettings& Settings)
	{
		return Settings.BlendInterpSpeed > 0.f
			? FMath::FInterpTo(CurrentAlpha, TargetAlpha, DeltaSeconds, Settings.BlendInterpSpeed)
			: TargetAlpha;
	}

	/** 래핑되는 각도 값을 최단 경로로 보간합니다. */
	FORCEINLINE float BlendAngleDegrees(float MuzzleAngle, float EyeAngle, float Alpha)
	{
		return MuzzleAngle + FMath::FindDeltaAngleDegrees(MuzzleAngle, EyeAngle) * Alpha;
	}

	/** 총구 기준 방향과 눈 위치 기준 방향을 같은 알파 규칙으로 보간합니다. */
	FORCEINLINE FVector BlendDirection(const FVector& MuzzleDirection, const FVector& EyeTargetDirection, float Alpha)
	{
		if (Alpha <= 0.f || EyeTargetDirection.IsNearlyZero())
		{
			return MuzzleDirection;
		}

		if (MuzzleDirection.IsNearlyZero())
		{
			return EyeTargetDirection.GetSafeNormal();
		}

		const FVector SafeEyeTargetDirection = EyeTargetDirection.GetSafeNormal();
		const FVector BlendedDirection = MuzzleDirection + (SafeEyeTargetDirection - MuzzleDirection) * Alpha;
		if (BlendedDirection.IsNearlyZero())
		{
			return Alpha >= 0.5f ? SafeEyeTargetDirection : MuzzleDirection;
		}

		return BlendedDirection.GetSafeNormal();
	}
}
