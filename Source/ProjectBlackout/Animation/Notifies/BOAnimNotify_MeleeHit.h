#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_MeleeHit.generated.h"

/**
 * 근접 공격 몽타주의 실제 타격 프레임에서 스윕 판정을 실행하는 노티파이.
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_MeleeHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
