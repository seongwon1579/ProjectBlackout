#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BlackoutInteractable.h"
#include "Interfaces/BlackoutPoolable.h"
#include "BlackoutDropItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UBOConsumableData;
class ABlackoutPlayerState;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * 드롭 아이템 종류를 나타내는 Enum
 */
UENUM(BlueprintType)
enum class EBlackoutDropItemType : uint8
{
	PrimaryAmmo UMETA(DisplayName = "Primary Weapon Ammo"),
	SecondaryAmmo UMETA(DisplayName = "Secondary Weapon Ammo"),
	Consumable UMETA(DisplayName = "Consumable Item")
};

/**
 * 풀링 및 상호작용을 지원하며 주무기/보조무기 탄약 및 무작위 소모품을 획득할 수 있는 월드 드롭 아이템 클래스입니다.
 * 지속형 나이아가라 이펙트를 직접 내장하여 생명 주기에 따라 정교하게 시각 효과를 제어합니다.
 */
UCLASS(Blueprintable)
class PROJECTBLACKOUT_API ABlackoutDropItem : public AActor, public IBlackoutInteractable, public IBlackoutPoolableInterface
{
	GENERATED_BODY()

public:
	ABlackoutDropItem();

	// IBlackoutInteractable 인터페이스 구현
	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void OnInteract_Implementation(AActor* Interactor) override;
	virtual FText GetInteractionPrompt_Implementation() const override;

	// IBlackoutPoolableInterface 인터페이스 구현
	virtual void OnSpawnFromPool_Implementation() override;
	virtual void OnReturnToPool_Implementation() override;

	// AActor 가상 함수 오버라이드 (가시성 제어 자동 동기화)
	virtual void SetActorHiddenInGame(bool bNewHidden) override;
	virtual void PostNetReceive() override;

	// 속성 리플리케이션 명세
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 드롭 아이템 타입 설정
	UFUNCTION(BlueprintCallable, Category = "Blackout|DropItem")
	void SetDropItemType(EBlackoutDropItemType NewType);

	/** 처치 계산기(ExecCalc) 등에서 드롭 스폰 시점에 보상 유형 및 보급 비율을 주입하기 위한 함수입니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|DropItem")
	void InitializeDropReward(EBlackoutDropItemType NewType, float NewSupplyRatio);

	UFUNCTION(BlueprintPure, Category = "Blackout|DropItem")
	EBlackoutDropItemType GetDropItemType() const { return DropItemType; }

	/** 현재 위치 아래의 바닥을 찾아 PickupMesh 하단이 바닥에 맞도록 위치를 보정합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|DropItem")
	void SnapToGround(AActor* IgnoreActor = nullptr);

protected:
	virtual void BeginPlay() override;

	// 수명 만료 시 호출되어 자신을 풀로 반환하는 함수 (서버 전용)
	void OnLifeTimeExpired();

	// 드롭 아이템의 보상 유형, 가시성, 획득 상태를 종합 분석하여 나이아가라 FX 재생을 동기화합니다. (클라이언트 동기화 무결성)
	void UpdateRewardVisual();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	// 이펙트 출력을 위한 나이아가라 컴포넌트 직접 탑재
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UNiagaraComponent> RewardEffectComponent;

	// 이 드롭 아이템의 유형 (클라이언트로 동기화됨)
	UPROPERTY(ReplicatedUsing = OnRep_DropItemType, EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem")
	EBlackoutDropItemType DropItemType = EBlackoutDropItemType::PrimaryAmmo;

	UFUNCTION()
	void OnRep_DropItemType();

	// 주무기 탄약 충전 비율 (최대 탄약 대비 비율, 예: 0.2f면 20% 충전)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|Ammo", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float PrimaryAmmoChargeRatio = 0.2f;

	// 보조무기 탄약 충전 비율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|Ammo", meta = (ClampMin = 0.0f, ClampMax = 1.0f))
	float SecondaryAmmoChargeRatio = 0.2f;

	// 무작위 지급할 블러드 루트 소모품 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|Consumable")
	TObjectPtr<UBOConsumableData> BloodRootData;

	// 무작위 지급할 굴 혈청 소모품 데이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|Consumable")
	TObjectPtr<UBOConsumableData> GulSerumData;

	// 바닥 잔존 수명 (초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|LifeTime", meta = (ClampMin = 1.0f))
	float LifeTime = 30.0f;

	// 바닥 스냅 라인트레이스 시작점의 상단 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|Placement", meta = (ClampMin = 0.0f))
	float GroundSnapTraceUpDistance = 120.0f;

	// 바닥 스냅 라인트레이스 하단 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|Placement", meta = (ClampMin = 0.0f))
	float GroundSnapTraceDownDistance = 500.0f;

	// PickupMesh 하단과 바닥 사이에 남길 여유 높이
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem|Placement", meta = (ClampMin = 0.0f))
	float GroundSnapClearance = 1.0f;

	// 동일 프레임 중복 획득을 방지하고 클라이언트 즉각 소멸을 보장하는 복제 플래그
	UPROPERTY(ReplicatedUsing = OnRep_bIsCollected, Transient)
	bool bIsCollected = false;

	UFUNCTION()
	void OnRep_bIsCollected();

	// 각 보상 종류별로 부착할 나이아가라 시스템 (에디터 블루프린트에서 세팅)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|DropItem|Visual")
	TObjectPtr<UNiagaraSystem> PrimaryAmmoFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|DropItem|Visual")
	TObjectPtr<UNiagaraSystem> SecondaryAmmoFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|DropItem|Visual")
	TObjectPtr<UNiagaraSystem> ConsumableFX;

private:
	// PostNetReceive에서 bHidden 변경 여부를 판단하기 위한 이전 값 캐시.
	// movement 복제 등 빈번한 net 업데이트마다 위젯/Niagara를 재설정하던 동작을 방지합니다.
	bool bLastReplicatedHidden = false;
	bool bHasCachedReplicatedHidden = false;

	FTimerHandle LifeTimeTimerHandle;

	// 획득 시 클라이언트로의 즉각 복제(ForceNetUpdate) 완료 후 안전하게 풀로 반환하기 위한 미세 지연 타이머 핸들
	FTimerHandle ReturnToPoolTimerHandle;

	// 상호작용 가능한지 여부를 확인하기 위해 플레이어 상태를 조회하는 헬퍼 함수
	bool TryResolvePlayerState(AActor* Interactor, ABlackoutPlayerState*& OutPlayerState) const;

	// 드롭 아이템이 가시화되는 시점에 로컬 플레이어의 포커스 상호작용 탐색을 즉시 갱신.
	// 100ms 스캔 인터벌 때문에 발생하는 위젯 표시 지연/실패를 막습니다.
	void NotifyLocalPlayerInteractableAvailable();
};
