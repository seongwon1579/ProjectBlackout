// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 허혁: 근접 공격 판정 구간을 NotifyState로 구현해 윈도우 동안 히트 판정을 수행
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "BOAnimNotifyState_MeleeAtkWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotifyState_MeleeAtkWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	
};
