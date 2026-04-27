// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "GameplayEffect.h"

#include "FActionData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FActionData
{
	GENERATED_BODY()

	// 주체
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Instigator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Target = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AActor*> HitTargets;

	// 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CostAmount = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownDuration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KnockbackStrength = 0.f;

	// 공간
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform SpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MovementDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed = 0.f;

	// GAS
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> CooldownEffectClass;

	// 실행할 핸들러 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ActionTags;

	// 실행 중 상태
	UPROPERTY(BlueprintReadOnly)
	bool bChainFailed = false;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag FailedTag;
};

