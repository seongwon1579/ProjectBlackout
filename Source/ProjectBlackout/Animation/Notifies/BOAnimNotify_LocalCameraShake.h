// ─── 구현 내역 ───────────────────────
//  - 최승현: 로컬 컨트롤러 화면에만 카메라 셰이크를 재생하는 포인트 노티파이를 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BOAnimNotify_LocalCameraShake.generated.h"

class UCameraShakeBase;

/**
 * 로컬 플레이어 화면에만 카메라 셰이크를 재생하는 포인트 노티파이입니다.
 * (몽타주는 전 머신에서 재생되므로 로컬 컨트롤 가드 필수)
 */
UCLASS()
class PROJECTBLACKOUT_API UBOAnimNotify_LocalCameraShake : public UAnimNotify
{
	GENERATED_BODY()

public:
	// 재생할 셰이크 에셋 (BP에서 UDefaultCameraShakeBase 서브클래스로 제작)
	UPROPERTY(EditAnywhere, Category = "Blackout|Camera")
	TSubclassOf<UCameraShakeBase> ShakeClass;

	UPROPERTY(EditAnywhere, Category = "Blackout|Camera", meta = (ClampMin = "0.0"))
	float Scale = 1.f;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
};
