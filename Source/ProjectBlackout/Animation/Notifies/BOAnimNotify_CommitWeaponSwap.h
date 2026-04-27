#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_CommitWeaponSwap.generated.h"

/**
 * 무기 스왑 몽타주 중간 프레임에서 실제 장착 전환을 확정하는 노티파이.
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_CommitWeaponSwap : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	virtual FString GetNotifyName_Implementation() const override;
};
