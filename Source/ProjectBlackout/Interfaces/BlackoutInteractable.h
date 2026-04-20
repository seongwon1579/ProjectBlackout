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
 * 화톳불(ABlackoutBonfire), 포털(ABlackoutPortal), 드랍 아이템 등이 구현.
 */
class PROJECTBLACKOUT_API IBlackoutInteractable
{
	GENERATED_BODY()

public:
	/** 상호작용 가능 여부. 기본값 true. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Interaction")
	bool CanInteract(AActor* Instigator) const;
	virtual bool CanInteract_Implementation(AActor* Instigator) const { return true; }

	/** 상호작용 실행. 서버에서 호출되어야 함. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Interaction")
	void OnInteract(AActor* Instigator);
	virtual void OnInteract_Implementation(AActor* Instigator) {}

	/** 월드 위젯에 표시할 상호작용 프롬프트 텍스트. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Blackout|Interaction")
	FText GetInteractionPrompt() const;
	virtual FText GetInteractionPrompt_Implementation() const { return FText::GetEmpty(); }
};
