#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_BossMeleeSweep.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBossMeleeSweepHitSignature, const FHitResult&, HitResult);

/**
 * 보스 근접 공격 판정 태스크.
 * 매 틱 StartSocket ~ EndSocket 사이를 구체 스윕하여 팔/턱 전체 길이를 커버한다.
 * 동일 액터 중복 피격은 HitActors 목록으로 방지한다.
 */
UCLASS()
class PROJECTBLACKOUT_API UAbilityTask_BossMeleeSweep : public UAbilityTask
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FBossMeleeSweepHitSignature OnHit;

	/**
	 * @param InStartSocketName 시작 소켓 
	 * @param InEndSocketName   끝 소켓   
	 * @param InSweepRadius     구체 반지름 (cm)
	 */
	static UAbilityTask_BossMeleeSweep* CreateSweepTask(
		UGameplayAbility* OwningAbility,
		FName InStartSocketName,
		FName InEndSocketName,
		float InSweepRadius);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	FName StartSocketName;
	FName EndSocketName;
	float SweepRadius = 30.f;

	TArray<TWeakObjectPtr<AActor>> HitActors;

	USkeletalMeshComponent* GetOwnerMesh() const;
};
