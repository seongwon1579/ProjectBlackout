// ─── 구현 내역 ───────────────────────
//  - 최승현: 풀링 적 사망 dissolve 연출 컴포넌트 — mesh DMI 구동 + 서버 타이머 완료 통지(풀 반환 연동)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "BODissolveComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UCurveFloat;
class UMaterialInstanceDynamic;

/**
 * 풀링 적의 사망 dissolve 연출을 캡슐화한 컴포넌트.
 * 소유 액터의 모든 MeshComponent 머티리얼을 DMI 화하여 DissolveAmount(1=멀쩡, 0=분해)를 구동한다.
 * 풀 반환 권위는 캐릭터에 있고, 이 컴포넌트는 서버 타이머 만료를 OnServerDissolveFinished 로만 통지한다.
 */
UCLASS(ClassGroup = (Blackout), meta = (BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBODissolveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBODissolveComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** 서버에서 사망 시 호출: Multicast 연출 재생 + DissolveDuration+Margin 후 완료 통지. */
	void PlayDissolve();

	/** 풀 재사용 시: Amount 1 복원 + Niagara 잔존 제거 + 타이머 클리어. */
	void ResetDissolve();

	/** 서버 타이머 만료(연출 완료 추정) 시 브로드캐스트. 캐릭터가 풀 반환에 사용. */
	FSimpleMulticastDelegate OnServerDissolveFinished;

protected:
	/** BP 에서 지정할 dissolve edge Niagara 시스템. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Dissolve")
	TObjectPtr<UNiagaraSystem> DissolveNiagaraSystem;

	/** 1→0 진행 커브. 길이는 DissolveDuration 과 맞춘다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Dissolve")
	TObjectPtr<UCurveFloat> DissolveCurve;

	/** dissolve 총 길이(초). 풀 반환 타이머 기준. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Dissolve")
	float DissolveDuration = 4.f;

	/** 클라 연출 완료 대기 마진. 서버가 이만큼 더 기다린 후 완료 통지. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Dissolve")
	float PoolReturnMargin = 0.5f;

	/** 머티리얼 scalar 파라미터 이름. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Dissolve")
	FName DissolveAmountParam = TEXT("DissolveAmount");

	/** Niagara User 파라미터 이름. SetVariableFloat 는 "User." 접두사 없이 변수명만. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Dissolve")
	FName NiagaraAmountParam = TEXT("DissolveAmount");

private:
	/** 소유 액터 mesh 슬롯별 DMI. BeginPlay 1회 생성, 풀 재사용 시 유지. */
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DissolveDMIs;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> DissolveNiagara;

	FTimeline DissolveTimeline;
	FTimerHandle PoolReturnTimerHandle;
	bool bIsDissolving = false;

	/** BeginPlay 1회: mesh 슬롯 DMI 생성 + Niagara 부착 + Timeline 바인딩. */
	void InitializeDissolve();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDissolve();

	UFUNCTION()
	void UpdateDissolve(float Value);

	UFUNCTION()
	void OnDissolveFinished();
};
