// ─── 구현 내역 ───────────────────────
//  - 허혁: 서버 권위 파괴/복원과 조각 물리 제어, 데미지 기반 파괴 조건 판정, VFX·사운드 큐를 갖춘 파괴 가능 기둥 액터
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BlackoutDamageable.h"
#include "BOBreakablePillarActor.generated.h"

class UChildActorComponent;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class USoundBase;

UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABOBreakablePillarActor : public AActor, public IBlackoutDamageable
{
	GENERATED_BODY()

public:
	ABOBreakablePillarActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버 권한에서 기둥을 파괴 상태로 전환합니다.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Blackout|Breakable")
	void BreakPillar();

	// 서버 권한에서 기둥을 초기 상태로 되돌립니다.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Blackout|Breakable")
	void ResetPillar();

	// 현재 액터 아래의 ChildActorComponent를 다시 수집합니다.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Blackout|Breakable")
	void RefreshBreakPieces();

	// 현재 기둥이 파괴 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category = "Blackout|Breakable")
	bool IsBroken() const { return bIsBroken; }

	// 기둥은 별도 부위 판정을 사용하지 않으므로 기본 태그만 반환합니다.
	virtual FGameplayTag GetHitPartTag(FName BoneName) const override;

	// Ravager 공격에서 들어온 피해 스펙만 받아 기둥 파괴를 트리거합니다.
	virtual void ReceiveDamageFromHitbox(const FGameplayEffectSpecHandle& SpecHandle, FName BoneName) override;

protected:
	// BP에서 하위 ChildActorComponent를 붙일 기준 루트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable")
	TObjectPtr<USceneComponent> Root;

	// Ravager 공격 히트박스가 직접 맞는 전용 판정 박스입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Collision")
	TObjectPtr<UBoxComponent> PillarHitbox;

	// 통짜 기둥으로 사용할 ChildActorComponent입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable")
	TObjectPtr<UChildActorComponent> WholeMesh;

	// WholeMesh 레퍼런스가 비어 있을 때 자동 탐색에 사용할 컴포넌트 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable")
	FName WholeMeshComponentName = TEXT("WholeMesh");

	// true면 WholeMesh를 제외한 나머지 ChildActorComponent를 자동으로 조각으로 수집합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable")
	bool bAutoCollectBreakPieces = true;

	// 자동 수집되거나 수동으로 채워진 조각 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable", meta = (EditCondition = "!bAutoCollectBreakPieces"))
	TArray<TObjectPtr<UChildActorComponent>> BreakPieces;

	// GameState의 DestroyedPillarIds와 연결할 선택적 식별자입니다.
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Blackout|Breakable")
	int32 PillarId = INDEX_NONE;

	// 디버그용 자동 파괴 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Debug")
	bool bAutoBreakForTest = false;

	// 자동 파괴까지 대기할 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Debug", meta = (EditCondition = "bAutoBreakForTest", ClampMin = "0.0"))
	float AutoBreakDelay = 2.0f;

	// 파괴 시 조각을 바깥으로 밀어내는 기본 힘입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Physics", meta = (ClampMin = "0.0"))
	float BreakImpulseStrength = 150.0f;

	// 파괴 시 조각에 추가할 위쪽 힘입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Physics", meta = (ClampMin = "0.0"))
	float BreakImpulseUpwardBoost = 35.0f;

	// 파괴 직후 조각의 선형 감쇠값입니다. 값이 높을수록 덜 미끄러집니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Physics", meta = (ClampMin = "0.0"))
	float PieceLinearDamping = 1.25f;

	// 파괴 직후 조각의 회전 감쇠값입니다. 값이 높을수록 덜 빙글돕니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Physics", meta = (ClampMin = "0.0"))
	float PieceAngularDamping = 2.0f;

	// 파괴된 조각이 화면에 유지되는 시간입니다. 0 이하면 자동 숨김을 사용하지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|Timing", meta = (ClampMin = "0.0"))
	float BrokenPieceVisibleDuration = 15.0f;

	// 기둥 파괴 순간에 재생할 더스트 BP 클래스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|VFX")
	TSubclassOf<AActor> BreakDustEffectClass;

	// 더스트 BP를 기둥 중심에서 얼마나 띄워서 재생할지 조정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|VFX")
	FVector BreakDustLocationOffset = FVector::ZeroVector;

	// 더스트 BP 회전을 기둥 회전 기준으로 보정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|VFX")
	FRotator BreakDustRotationOffset = FRotator::ZeroRotator;

	// 외부 BP가 자체 수명을 정리하지 않을 때 사용할 선택적 수명입니다. 0 이하면 그대로 둡니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|VFX", meta = (ClampMin = "0.0"))
	float BreakDustLifeSpan = 0.0f;

	// 기둥 파괴 순간에 재생할 Sound Cue 또는 Sound Base입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|SFX")
	TObjectPtr<USoundBase> BreakSound;

	// 파괴 사운드를 기둥 중심에서 얼마나 띄워서 재생할지 조정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|SFX")
	FVector BreakSoundLocationOffset = FVector::ZeroVector;

	// 파괴 사운드의 볼륨 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|SFX", meta = (ClampMin = "0.0"))
	float BreakSoundVolumeMultiplier = 1.0f;

	// 파괴 사운드의 피치 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Breakable|SFX", meta = (ClampMin = "0.0"))
	float BreakSoundPitchMultiplier = 1.0f;

	// 실제 파괴 여부를 네트워크로 동기화합니다.
	UPROPERTY(ReplicatedUsing = OnRep_IsBroken, VisibleInstanceOnly, BlueprintReadOnly, Category = "Blackout|Breakable")
	bool bIsBroken = false;

	// 파괴된 조각이 자동으로 숨겨졌는지 네트워크로 동기화합니다.
	UPROPERTY(ReplicatedUsing = OnRep_AreBrokenPiecesHidden, VisibleInstanceOnly, BlueprintReadOnly, Category = "Blackout|Breakable")
	bool bAreBrokenPiecesHidden = false;

	UFUNCTION()
	void OnRep_IsBroken();

	UFUNCTION()
	void OnRep_AreBrokenPiecesHidden();

	// 파괴 순간의 더스트 연출을 현재 접속 중인 모든 클라이언트에 동기화합니다.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayBreakDustEffect();

private:
	// Reset 시 ChildActorComponent 기준 원래 상대 위치로 복원하기 위한 조각 초기 트랜스폼입니다.
	UPROPERTY()
	TArray<FTransform> InitialPieceTransforms;

	// 디버그 자동 파괴용 타이머입니다.
	FTimerHandle AutoBreakTimerHandle;

	// 파괴된 조각을 일정 시간 뒤 숨기기 위한 타이머입니다.
	FTimerHandle HideBrokenPiecesTimerHandle;

	// 현재 파괴 상태를 컴포넌트 시각/물리 상태에 반영합니다.
	void ApplyCurrentState();

	// 서버에서 파괴된 조각을 일정 시간 뒤 숨기도록 예약합니다.
	void ScheduleBrokenPieceHide();

	// 서버에서 파괴된 조각을 숨김 상태로 전환합니다.
	void HideBrokenPieces();

	// 통짜 메시에 보임/숨김 상태를 적용합니다.
	void ApplyWholeMeshState(bool bVisible);

	// 조각 하나에 보임/충돌/물리 상태를 적용합니다.
	void ApplyPieceState(UChildActorComponent* PieceComponent, bool bVisible, bool bEnablePhysics);

	// 조각의 초기 트랜스폼을 캐시합니다.
	void CacheInitialPieceTransforms();

	// 조각을 초기 위치와 속도로 되돌립니다.
	void RestorePieceTransforms();

	// 파괴 시 조각에 바깥 방향 임펄스를 적용합니다.
	void ApplyBreakImpulse();

	// 현재 월드에서 더스트 BP를 실제로 스폰합니다.
	void PlayBreakDustEffect();

	// 현재 월드에서 파괴 사운드를 실제로 재생합니다.
	void PlayBreakSound();

	// ChildActor 내부에서 주 물리 컴포넌트를 찾습니다.
	UPrimitiveComponent* ResolvePrimitiveFromChild(UChildActorComponent* ChildActorComponent) const;

	// ChildActorComponent가 보유한 현재 월드 트랜스폼에 자식 액터를 다시 맞춥니다.
	void SyncChildActorToComponent(UChildActorComponent* ChildActorComponent) const;

	// 물리 또는 이동이 필요한 컴포넌트가 Movable 상태인지 보장합니다.
	void EnsurePrimitiveIsMovable(UPrimitiveComponent* PrimitiveComponent) const;

	// 현재 피해 스펙의 출처가 Ravager인지 판정합니다.
	bool IsDamageSpecFromRavager(const FGameplayEffectSpecHandle& SpecHandle) const;

	// GameState의 파괴 기록 배열을 갱신합니다.
	void UpdateDestroyedPillarState(bool bDestroyed) const;

	// BeginPlay 시 이미 파괴된 기둥인지 확인합니다.
	bool IsMarkedDestroyedInGameState() const;
};
