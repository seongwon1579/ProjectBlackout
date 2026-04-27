#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "BOAnimNotifyState_MeleeComboWindow.generated.h"

/**
 * 근접 공격 콤보 입력 허용 구간을 열고 닫는 노티파이 스테이트.
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotifyState_MeleeComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
