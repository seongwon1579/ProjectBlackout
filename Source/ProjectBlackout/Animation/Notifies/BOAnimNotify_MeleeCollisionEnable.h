#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_MeleeCollisionEnable.generated.h"

/**
 * 근접 공격 데미지 히트박스를 활성화하는 포인트 노티파이.
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_MeleeCollisionEnable : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
