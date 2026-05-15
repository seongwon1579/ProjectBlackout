#pragma once

#include "CoreMinimal.h"
#include "BlackoutHUDTypes.generated.h"

UENUM(BlueprintType)
enum class EBlackoutTrajectoryVisualState : uint8
{
	// 정상적으로 예측된 궤적 구간입니다.
	Normal,

	// 충격 신관 활성 거리보다 가까워 경고 색상으로 표시할 구간입니다.
	FuseInactive,

	// 카메라 기준으로 착탄점이 가려진 상태를 전달할 때 사용합니다.
	Occluded
};

UENUM(BlueprintType)
enum class EBlackoutInteractionPromptState : uint8
{
	Hidden,
	Available,
	MissingRequirement,
	Busy,
	InProgress
};

using EBlackoutRevivePromptState = EBlackoutInteractionPromptState;

USTRUCT(BlueprintType)
struct FBlackoutTrajectoryPointData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	/** 총구에서 이 포인트까지 예측 궤적을 따라 누적 이동한 거리입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	float DistanceFromMuzzle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	EBlackoutTrajectoryVisualState VisualState = EBlackoutTrajectoryVisualState::Normal;
};

USTRUCT(BlueprintType)
struct FBlackoutImpactIndicatorData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bIsVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bHasBlockingHit = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bTargetMismatch = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bUsesProjectilePrediction = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bIsOccludedFromCamera = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bProjectileImpactFuseInactive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector TraceEndLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	float DistanceFromMuzzle = 0.0f;

	/** 투사체 무기일 때 HUD가 그릴 예측 궤적 포인트입니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	TArray<FBlackoutTrajectoryPointData> TrajectoryPoints;

	/** 현재 탄퍼짐을 0(기본)~1(최대) 범위로 정규화한 값. 인디케이터 크기·크로스헤어 확장에 사용합니다. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	float SpreadNormalized = 0.0f;
};

USTRUCT(BlueprintType)
struct FBlackoutInteractionPromptData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bIsVisible = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bShowProgress = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	bool bIsStatusError = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	float ProgressNormalized = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FVector2D ScreenPosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	EBlackoutInteractionPromptState State = EBlackoutInteractionPromptState::Hidden;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FText PromptText;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD")
	FText StatusText;
};

using FBlackoutRevivePromptData = FBlackoutInteractionPromptData;
