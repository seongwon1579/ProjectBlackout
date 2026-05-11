#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"

#include "AbilityTask_BossMeleeHitbox.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBossMeleeHitboxHitSignature, const FHitResult&, HitResult);

/**
 * 보스 근접 공격 판정 태스크.
 * 지정한 PrimitiveComponent의 콜리전을 활성화하고 BeginOverlap으로 히트를 감지한다.
 * 동일 액터 중복 피격은 HitActors 목록으로 방지한다.
 */
UCLASS()
class PROJECTBLACKOUT_API UAbilityTask_BossMeleeHitbox : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FBossMeleeHitboxHitSignature OnHit;

	static UAbilityTask_BossMeleeHitbox* InitCustomTask(
		UGameplayAbility* OwningAbility,
		UPrimitiveComponent* InHitbox);

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	virtual void TickTask(float DeltaTime) override;

private:
	TWeakObjectPtr<UPrimitiveComponent> Hitbox;
	TArray<TWeakObjectPtr<AActor>> HitActors;

	UFUNCTION()
	void OnHitboxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	UPrimitiveComponent* HitboxComp;
};
