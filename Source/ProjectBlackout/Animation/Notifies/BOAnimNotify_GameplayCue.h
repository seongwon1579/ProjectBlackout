// ─── 구현 내역 ───────────────────────
//  - 김민영: 소켓·오프셋 기준으로 소유 액터 ASC에 GameplayCue를 실행하는 노티파이를 구현
// ──────────────────────────────────────

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

	/** GameplayCue의 시작 위치와 회전(Normal)을 가져올 소켓 이름입니다. (지정하지 않으면 액터 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|GameplayCue")
	FName SocketName = NAME_None;

	/** 회전 오프셋입니다. Normal 벡터에 이 회전값이 가해집니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|GameplayCue")
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** 로컬 위치 오프셋입니다. 소켓/캐릭터의 로컬 좌표계 기준으로 적용됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|GameplayCue")
	FVector LocationOffset = FVector::ZeroVector;
};
