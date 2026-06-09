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

/**
 * 로컬 플레이어 HUD가 현재 진입한 모드입니다.
 * Combat: 일반 전투 HUD. Downed*: 다운 상태 HUD. Spectator: 완전 사망 후 관전.
 */
UENUM(BlueprintType)
enum class EBlackoutHUDMode : uint8
{
	// 일반 생존 상태 또는 부활 성공 직후 기본 전투 HUD.
	Combat,

	// 다운 상태에서 완전 사망까지 남은 시간을 보여줄 때.
	DownedDeathTimer,

	// 다운 상태에서 아군이 부활을 시도해 사망 타이머가 일시정지된 동안.
	DownedReviveTimer,

	// 완전 사망 후 관전 HUD로 전환된 상태.
	Spectator
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

/**
 * 다운 상태 위젯이 화면에 표시할 사망/부활 타이머 데이터입니다.
 * 컨트롤러가 서버 권위 다운 사망 타이머와 부활 진행률을 매 틱 폴링해 채웁니다.
 */
USTRUCT(BlueprintType)
struct FBlackoutDownedStateHUDData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	EBlackoutHUDMode HUDMode = EBlackoutHUDMode::Combat;

	/** 표시할 남은 시간(초). 사망 타이머 또는 부활 잔여 시간. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	float RemainingTime = 0.0f;

	/** 프로그래스 바 비율 계산용 총 지속 시간(초). */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	float TotalDuration = 0.0f;

	/** 0(시작)~1(완료) 범위의 진행 비율입니다. 사망 타이머는 0→1로 누적, 부활은 0→1로 진행. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	float ProgressNormalized = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	FText StatusText;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Downed")
	bool bIsVisible = false;
};
