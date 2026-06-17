// ─── 구현 내역 ───────────────────────
//  - 허혁: 근접 콤보 입력 윈도우를 여는 포인트 노티파이를 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_MeleeComboWindowOpen.generated.h"

/**
 * 근접 공격 콤보 입력 윈도우를 여는 포인트 노티파이.
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_MeleeComboWindowOpen : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
