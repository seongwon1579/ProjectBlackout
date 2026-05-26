#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_GameplayCue.generated.h"

/**
 * 몽타주 특정 프레임에서 소유 액터의 ASC를 통해 지정된 GameplayCue를 실행하는 노티파이.
 */
UCLASS(meta = (DisplayName = "Send Gameplay Cue"))
class PROJECTBLACKOUT_API UBOAnimNotify_GameplayCue : public UAnimNotify
{
	GENERATED_BODY()

public:
	UBOAnimNotify_GameplayCue();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	/** 실행할 GameplayCue 태그입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|GameplayCue", meta = (Categories = "GameplayCue"))
	FGameplayTag GameplayCueTag;
};
