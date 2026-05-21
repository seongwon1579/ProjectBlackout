#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/BlackoutInteractable.h"
#include "Interfaces/BlackoutPoolable.h"
#include "GameplayTagContainer.h"
#include "BlackoutDropItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class UBOConsumableData;
class ABlackoutPlayerState;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<USphereComponent> InteractionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UStaticMeshComponent> PickupMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Components")
	TObjectPtr<UWidgetComponent> InteractionWidget;

	// 이 드롭 아이템의 유형
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|DropItem")
	EBlackoutDropItemType DropItemType = EBlackoutDropItemType::PrimaryAmmo;

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

	// 동일 프레임 중복 획득을 방지하는 서버용 획득 여부 플래그
	UPROPERTY(Transient)
	bool bIsCollected = false;

private:
	FTimerHandle LifeTimeTimerHandle;

	// 상호작용 가능한지 여부를 확인하기 위해 플레이어 상태를 조회하는 헬퍼 함수
	bool TryResolvePlayerState(AActor* Interactor, ABlackoutPlayerState*& OutPlayerState) const;
};
