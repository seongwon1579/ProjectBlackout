// ─── 구현 내역 ───────────────────────
//  - 김민영: 근접 콤보 데이터 정의 — 콤보 섹션별 입력 윈도우·데미지 배율, 입력 동기화 v2용 클라/서버 버퍼·grace·핑 보정·timestamp 검증 파라미터
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "BOMeleeComboData.generated.h"

class UAnimMontage;

/**
 * 콤보 섹션 한 단계의 시각 정의.
 * 모든 시각은 섹션 시작을 기준으로 한 절대 초(server world time) 단위입니다.
 */
USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutComboSectionDef
{
	GENERATED_BODY()

	/** 몽타주의 섹션 이름. 예: "Attack1_Start" */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	FName SectionName = NAME_None;

	/** 섹션 시작 후 콤보 입력 윈도우가 열리는 시각(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = 0.0))
	float WindowOpenAtSeconds = 0.f;

	/** 섹션 시작 후 콤보 입력 윈도우가 닫히는 시각(초). WindowOpen 이후여야 합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = 0.0))
	float WindowCloseAtSeconds = 0.f;

	/** 해당 콤보 단계에서 적용할 데미지 배율. 기본값 1.0 (100%) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat", meta = (ClampMin = 0.0))
	float DamageMultiplier = 1.f;
};

/**
 * 플레이어 근접 콤보 데이터 (TDD v5 §4.1 v2).
 * 무기/캐릭터별 콤보 정의를 데이터 주도적으로 관리합니다.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBOMeleeComboData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 콤보 시퀀스가 재생할 몽타주 (모든 콤보 섹션이 이 몽타주 안에 있어야 함). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UAnimMontage> MeleeMontage;

	/** 콤보 단계 순서대로의 섹션 정의. 첫 항목이 시작 섹션입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TArray<FBlackoutComboSectionDef> ComboSections;

	/** 근접 타격 시 적용할 데미지 GameplayEffect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/** 클라이언트가 윈도우 열리기 전 도착한 입력을 임시 보관하는 길이(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Buffer", meta = (ClampMin = 0.0))
	float ClientInputBufferDuration = 0.25f;

	/** 서버가 윈도우 열리기 전 도착한 입력을 보관하는 길이(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Buffer", meta = (ClampMin = 0.0))
	float ServerReceiveBufferDuration = 0.15f;

	/** 콤보 윈도우 종료 후 추가로 허용할 late grace 의 베이스 길이(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Grace", meta = (ClampMin = 0.0))
	float BaseComboInputGraceDuration = 0.05f;

	/** Ping 변동 흡수용 마진(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Grace", meta = (ClampMin = 0.0))
	float ComboInputJitterMargin = 0.04f;

	/** 동적 grace 의 상한(초). 핑이 매우 높아도 이 이상 늘어나지 않습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Grace", meta = (ClampMin = 0.0))
	float MaxComboInputGraceDuration = 0.15f;

	/** 핑 보정 계수. grace 에 더해질 RTT 비율. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Grace", meta = (ClampMin = 0.0))
	float ComboInputPingScale = 0.5f;

	/** 입력 timestamp 의 최대 허용 과거 age(초). 이보다 오래된 timestamp 는 clamp. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Validation", meta = (ClampMin = 0.0))
	float MaxComboInputTimestampAge = 0.75f;

	/** 입력 timestamp 의 최대 허용 미래 tolerance(초). 이보다 미래 timestamp 는 clamp. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat|Validation", meta = (ClampMin = 0.0))
	float MaxComboInputFutureTolerance = 0.1f;
};
