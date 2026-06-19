// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 허혁: 회피 무적(I-Frame) 구간을 NotifyState로 구현해 윈도우 동안 피격 무효 상태를 처리
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "BOAnimNotifyState_DodgeIFrame.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotifyState_DodgeIFrame : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override;
};
