// ─── 구현 내역 ───────────────────────
//  - 김민영: 지정한 태그로 소유 액터 ASC에 GameplayEvent를 발송하는 범용 노티파이를 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_GameplayEvent.generated.h"

/**
 * 몽타주 특정 프레임에서 소유 액터의 ASC에 GameplayEvent를 발송하는 범용 노티파이.
 */
UCLASS(meta = (DisplayName = "Send Gameplay Event"))
class PROJECTBLACKOUT_API UBOAnimNotify_GameplayEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|GameplayEvent")
	FGameplayTag EventTag;
};
