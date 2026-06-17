// ─── 구현 내역 ───────────────────────
//  - 김민영: 캡슐 기반 히트박스 컴포넌트(부위 태그·데미지 배율, 데미지 Spec 수신) 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Components/CapsuleComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "BlackoutHitboxComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBlackoutHitboxComponent : public UCapsuleComponent
{
	GENERATED_BODY()

public:	
	UBlackoutHitboxComponent();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayTag GetPartTag() const { return PartTag; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetDamageMultiplier() const { return DamageMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void ReceiveDamageSpec(const FGameplayEffectSpecHandle& SpecHandle);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	FGameplayTag PartTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	FName AttachedBoneName;
};
