// ─── 구현 내역 ───────────────────────
//  - 김민영: [E] 상호작용 공용 인터페이스와 프롬프트 텍스트 조회(GetInteractionPrompt) 정의
//  - 최승현: 체크포인트 상호작용용 가능여부 판정/실행(CanInteract·OnInteract) 시그니처 정립
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BlackoutInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UBlackoutInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * [E] 상호작용 가능한 오브젝트 공용 인터페이스.
 * 체크포인트(ABlackoutCheckpoint), 포털(ABlackoutPortal), 드랍 아이템 등이 구현.
 */
class PROJECTBLACKOUT_API IBlackoutInteractable
{
	GENERATED_BODY()

public:
	/** 상호작용 가능 여부. 기본값 true. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Interaction")
	bool CanInteract(AActor* Interactor) const;
	virtual bool CanInteract_Implementation(AActor* Interactor) const { return true; }

	/** 상호작용 실행. 서버에서 호출되어야 함. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Interaction")
	void OnInteract(AActor* Interactor);
	virtual void OnInteract_Implementation(AActor* Interactor) {}

	/** 월드 위젯에 표시할 상호작용 프롬프트 텍스트. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Interaction")
	FText GetInteractionPrompt() const;
	virtual FText GetInteractionPrompt_Implementation() const { return FText::GetEmpty(); }
};
