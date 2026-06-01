#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/BlackoutHUDTypes.h"
#include "BlackoutPartyTypes.generated.h"

class ABlackoutPlayerState;

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutPartyMemberStatusData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	TObjectPtr<ABlackoutPlayerState> PlayerState = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FText DisplayName = FText::GetEmpty();

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	FGameplayTag SelectedClassTag;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float CurrentHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float MaxHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float NormalizedHealth = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsDowned = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsReviveInteractionActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsLocalPlayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	bool bIsValid = false;

	/** 팀원이 현재 어떤 다운/관전 모드에 있는지. Combat이면 타이머 표시 없음. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	EBlackoutHUDMode HUDMode = EBlackoutHUDMode::Combat;

	/** 사망 또는 부활까지 남은 시간(초). HUDMode가 타이머 모드일 때만 유효. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float TimerRemainingTime = 0.0f;

	/** 타이머의 총 지속 시간(초). 0이면 비율 계산 불가. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float TimerTotalDuration = 0.0f;

	/** 0(시작)~1(완료). 사망 타이머는 줄어들수록 1에 가까워지고, 부활은 진행될수록 1에 가까워짐. */
	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Party")
	float TimerProgressNormalized = 0.0f;
};
