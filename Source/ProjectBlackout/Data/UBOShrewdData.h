// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: Shrewd(중간보스) 기본 데이터 에셋 정의 — 최대 체력·이동속도·부여 어빌리티 목록
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UBOShrewdData.generated.h"

class UGameplayAbility;
/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UUBOShrewdData : public UDataAsset
{
	GENERATED_BODY()

public:
	bool IsValid(){ return MaxHealth > 0.f && MovementSpeed > 0.f && !Name.IsEmpty(); }
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	FText Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MaxHealth = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float MovementSpeed = 400.f;
	
	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category = "Blackout|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;
};
