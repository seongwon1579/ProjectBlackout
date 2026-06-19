// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 지정 태그로 소유 액터 ASC에 GameplayEvent를 발송하는 공용 적 노티파이를 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_AbilityEvent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_AbilityEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	UPROPERTY(EditAnywhere, Category = "Event", meta = (Categories = "Event"))
	FGameplayTag EventTag;
};
