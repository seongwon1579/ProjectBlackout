// ─── 구현 내역 ───────────────────────
//  - 허혁: 회피 체인 입력 윈도우를 여는 포인트 노티파이를 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_DodgeChainWindowOpen.generated.h"

/**
 * 회피 체인 입력 윈도우를 여는 포인트 노티파이입니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_DodgeChainWindowOpen : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
