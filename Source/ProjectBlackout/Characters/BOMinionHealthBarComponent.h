// ─── 구현 내역 ───────────────────────
//  - 최승현: 미니언 머리 위 체력바를 분리 가능한 부속 컴포넌트로 캡슐화(ASC 바인딩 + 동적 부착/표시)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BOMinionHealthBarComponent.generated.h"

class UAbilitySystemComponent;
class UBlackoutMinionHealthBarWidget;
class UWidgetComponent;

/**
 * 미니언 머리 위 월드 위젯 체력바. 내부에 UWidgetComponent 를 보유하고 owner 의
 * RootComponent 에 동적으로 부착한다. dissolve 컴포넌트와 같은 *분리 가능한 부속*
 * 패턴이라 미니언 / 더미 / 미래 다른 적 어디든 부착해서 사용 가능.
 */
UCLASS(ClassGroup = "Blackout", meta = (BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBOMinionHealthBarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBOMinionHealthBarComponent();

	/** 위젯 클래스 인스턴스화 + ASC 와 바인딩 + 표시. 일반적으로 BeginPlay / OnSpawnFromPool 에서 호출. */
	void InitializeFromASC(UAbilitySystemComponent* AbilitySystemComponent);

	/** 사망 / 풀 반환 시 강제 숨김. */
	void Hide();

protected:
	virtual void OnRegister() override;

	/** 내부 위젯 컴포넌트. owner 의 RootComponent 에 동적 부착. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|UI")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	/** 머리 위 표시 높이 (owner 의 root 기준 Z offset). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|UI", meta = (ClampMin = "0.0"))
	float HeightOffset = 120.0f;

	/** 체력바 월드 위젯의 렌더 크기. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|UI", meta = (ClampMin = "1.0"))
	FVector2D DrawSize = FVector2D(120.0f, 12.0f);

	/** 체력바 위젯 클래스. BP 인스턴스에서 WBP_MinionHealthBar 지정. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|UI")
	TSubclassOf<UBlackoutMinionHealthBarWidget> WidgetClass;

private:
	void ApplyLayout();
};
