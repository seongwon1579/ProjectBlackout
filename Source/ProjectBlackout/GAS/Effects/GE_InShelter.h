// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: 쉘터존 체류 중 State.InShelter 태그를 부여하는 GE(비복제 Loose 태그를 GE-with-granted-tag로 전환)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GE_InShelter.generated.h"

/**
 *  쉘더 존 안에 있는 동안 부여되는 State.InShelter 태그 GE
 */
UCLASS()
class PROJECTBLACKOUT_API UGE_InShelter : public UGameplayEffect
{
	GENERATED_BODY()
	
public:
	UGE_InShelter();
};
