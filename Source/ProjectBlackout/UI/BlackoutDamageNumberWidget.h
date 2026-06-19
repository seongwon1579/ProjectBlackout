// ─── 구현 내역 ───────────────────────
//  - 허혁: 데미지 숫자 추적·로컬 예측 출력 위젯 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutDamageNumberWidget.generated.h"

class APlayerController;
class UCanvasPanelSlot;

/**
 * 월드 위치를 따라다니는 데미지 숫자 위젯의 C++ 베이스 클래스입니다.
 * 실제 텍스트 스타일과 애니메이션은 블루프린트에서 담당하고, 위치 추적만 C++에서 처리합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutDamageNumberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeDamageNumber(
		float InDamageAmount,
		bool bInIsCritical,
		const FVector& InWorldLocation,
		const FVector2D& InRandomScreenOffset);

	void SetCanvasSlot(UCanvasPanelSlot* InCanvasSlot);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|DamageNumber")
	float DamageAmount = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|DamageNumber")
	bool bIsCritical = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|DamageNumber")
	FVector TargetWorldLocation = FVector::ZeroVector;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|DamageNumber")
	FVector2D RandomScreenOffset = FVector2D::ZeroVector;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Damage Number Initialized"), Category = "Blackout|DamageNumber")
	void ReceiveDamageNumberInitialized(float InDamageAmount, bool bInIsCritical);

private:
	void RefreshProjectedScreenPosition() const;

	UPROPERTY(Transient)
	TWeakObjectPtr<UCanvasPanelSlot> CachedCanvasPanelSlot;

	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> CachedPlayerController;
};
