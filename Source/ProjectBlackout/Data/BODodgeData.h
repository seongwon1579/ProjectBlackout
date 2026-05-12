#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffect.h"
#include "BODodgeData.generated.h"

class UAnimMontage;

/**
 * 플레이어 구르기/연속 구르기 데이터 (TDD v5 §4.1 v2).
 * 무기/캐릭터별 회피 정의를 데이터 주도적으로 관리합니다.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBODodgeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 회피 몽타주. 체인 시에도 동일 몽타주를 처음부터 다시 재생합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge")
	TObjectPtr<UAnimMontage> DodgeMontage;

	/** 스태미나 소모량. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Cost", meta = (ClampMin = 0.0))
	float StaminaCost = 25.f;

	/** 전방/방향 회피 시 가속 강도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Motion", meta = (ClampMin = 0.0))
	float DodgeStrength = 900.f;

	/** 백스텝 시 가속 강도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Motion", meta = (ClampMin = 0.0))
	float BackstepStrength = 650.f;

	/** 회피 시 살짝 띄울 수직 임펄스. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Motion", meta = (ClampMin = 0.0))
	float UpwardImpulse = 120.f;

	/** 몽타주 길이가 유효하지 않을 때 사용할 fallback 회피 지속 시간. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Motion", meta = (ClampMin = 0.01))
	float DodgeDuration = 0.35f;

	/** 회피 시작 후 체인 입력 윈도우가 열리는 시각(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Chain", meta = (ClampMin = 0.0))
	float ChainWindowOpenAtSeconds = 0.20f;

	/** 회피 시작 후 체인 입력 윈도우가 닫히는 시각(초). WindowOpen 이후여야 합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Chain", meta = (ClampMin = 0.0))
	float ChainWindowCloseAtSeconds = 0.40f;

	/** 클라이언트가 윈도우 열리기 전 도착한 입력을 임시 보관하는 길이(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Buffer", meta = (ClampMin = 0.0))
	float ClientInputBufferDuration = 0.15f;

	/** 서버가 윈도우 열리기 전 도착한 입력을 보관하는 길이(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Buffer", meta = (ClampMin = 0.0))
	float ServerReceiveBufferDuration = 0.10f;

	/** 체인 윈도우 종료 후 추가로 허용할 late grace 의 베이스 길이(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Grace", meta = (ClampMin = 0.0))
	float BaseChainInputGraceDuration = 0.05f;

	/** Ping 변동 흡수용 마진(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Grace", meta = (ClampMin = 0.0))
	float ChainInputJitterMargin = 0.04f;

	/** 동적 grace 의 상한(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Grace", meta = (ClampMin = 0.0))
	float MaxChainInputGraceDuration = 0.15f;

	/** 핑 보정 계수. grace 에 더해질 RTT 비율. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Grace", meta = (ClampMin = 0.0))
	float ChainInputPingScale = 0.5f;

	/** 입력 timestamp 의 최대 허용 과거 age(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Validation", meta = (ClampMin = 0.0))
	float MaxChainInputTimestampAge = 0.75f;

	/** 입력 timestamp 의 최대 허용 미래 tolerance(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Dodge|Validation", meta = (ClampMin = 0.0))
	float MaxChainInputFutureTolerance = 0.1f;
};
