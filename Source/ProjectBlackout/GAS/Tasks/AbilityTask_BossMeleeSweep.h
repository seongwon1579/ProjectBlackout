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
	 * @param InMeshOverride    소켓 검색 대상 Mesh (nullptr이면 OwnerActor가 ACharacter일 때 GetMesh() 폴백)
	 */
	static UAbilityTask_BossMeleeSweep* CreateSweepTask(
		UGameplayAbility* OwningAbility,
		FName InStartSocketName,
		FName InEndSocketName,
		float InSweepRadius,
		UMeshComponent* InMeshOverride = nullptr);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	FName StartSocketName;
	FName EndSocketName;
	float SweepRadius = 30.f;
	int32 SweepSamples = 5;

	TWeakObjectPtr<UMeshComponent> MeshOverride;

	TArray<TWeakObjectPtr<AActor>> HitActors;

	bool bEnableBossDebug = false;
	bool bFirstTick = true;
	FVector PrevStartLoc = FVector::ZeroVector;
	FVector PrevEndLoc   = FVector::ZeroVector;

	UMeshComponent* GetOwnerMesh() const;
	bool DoSweep(const FVector& From, const FVector& To, const FCollisionQueryParams& Params, FHitResult& OutHit) const;
};
