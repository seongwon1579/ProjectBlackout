// ─── 구현 내역 ───────────────────────
//  - 김민영: 무기 데미지 파이프라인 검증용 더미 — 피격 데미지 수신/화면 출력 + 체력 리셋 디버그
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "BlackoutMinionCharacter.h"
#include "GameplayEffectTypes.h"
#include "BlackoutDamageTestDummy.generated.h"

/**
 * 무기 데미지 파이프라인 검증용 깡통 더미.
 * 에디터에 배치한 뒤 FireWeapon/GE_Damage/Hitbox 경로를 확인하는 용도로 사용합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABlackoutDamageTestDummy : public ABlackoutMinionCharacter
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName) override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Debug")
	void ResetDummyHealth();

	UFUNCTION(BlueprintPure, Category = "Blackout|Debug")
	float GetCurrentHealth() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Debug", meta = (ClampMin = 1.f))
	float TestMaxHealth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Debug")
	bool bPrintDamageToScreen = true;

	virtual void OnDeath() override;
};
