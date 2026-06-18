// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager 기본 스탯 데이터 에셋 정의(이름·최대 체력·이동속도 + 유효성 검사)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BORavagerStatData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBORavagerStatData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	bool IsValid(){ return MaxHealth > 0.f && MovementSpeed > 0.f && !Name.IsEmpty(); }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	FText Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MaxHealth = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MovementSpeed = 300.f;
};
