#include "AI/BOAICalcHelper.h"
#include "GameFramework/Actor.h"

bool UBOAICalcHelper::IsWithinRange(const AActor* Self, const AActor* Target, float Range)
{
	if (!Self || !Target) return false;
	// 제곱근 계산만하여 범위 내인지 확인
	return FVector::DistSquared2D(Self->GetActorLocation(), Target->GetActorLocation())
	       <= FMath::Square(Range);
}

float UBOAICalcHelper::GetDistance2D(const AActor* A, const AActor* B)
{
	if (!A || !B) return 0.f;
	
	// 실제 거리 계산
	return FVector::Dist2D(A->GetActorLocation(), B->GetActorLocation());
}

EBOTurnDirection UBOAICalcHelper::GetTurnDirection(const AActor* Self, const AActor* Target,
                                                     float Threshold, float& OutAngleDelta)
{
	OutAngleDelta = 0.f;
	if (!Self || !Target) return EBOTurnDirection::None;

    // GetSafeNormal2D - 방향 벡터만 계산
	const FVector ToTarget = (Target->GetActorLocation() - Self->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero()) return EBOTurnDirection::None;

	// Yaw 기준 Forward/Right 벡터로 부호 있는 각도 계산 (-180 ~ 180)
	
	// Yaw 방향만 남김
	const FRotator YawRot(0.f, Self->GetActorRotation().Yaw, 0.f);
	
	// 현재 Yaw 방향이 현재 앞을 바라보는 벡터
	const FVector Fwd   = YawRot.Vector();
	
	// 현재 벡터의 회전 행렬에서 Y(오른쪽) 방향벡터 계산
	const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	// Dot 계산하여 현재 X와 Y 계산
	const float Cos = FVector::DotProduct(Fwd, ToTarget);
	const float Sin = FVector::DotProduct(Right, ToTarget);
	
	// 라디안값으로 변환
	OutAngleDelta = FMath::RadiansToDegrees(FMath::Atan2(Sin, Cos));

	// Treshold를 넘는경우 회전 방향을 산출
	if (FMath::Abs(OutAngleDelta) < Threshold) return EBOTurnDirection::None;
	return OutAngleDelta > 0.f ? EBOTurnDirection::Right : EBOTurnDirection::Left;
}