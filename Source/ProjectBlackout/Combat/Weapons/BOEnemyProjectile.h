// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 조성원: 적 발사체 베이스(서버 권위 이동·리플리케이션, 박스 콜리전 오버랩/피직스 분기, 스탯 기반 데미지) 구현
//  - 허혁: 오브젝트 풀 프리로드 및 스턴 게이지 적용 연동
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/BORavagerPatternData.h"
#include "Interfaces/BlackoutPoolable.h"

#include "BOEnemyProjectile.generated.h"

class UNiagaraComponent;
class UBoxComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTBLACKOUT_API ABOEnemyProjectile : public AActor, public IBlackoutPoolableInterface
{
	GENERATED_BODY()
	
public:	
	ABOEnemyProjectile();

	virtual void OnSpawnFromPool_Implementation() override;
	virtual void OnReturnToPool_Implementation() override;
	
	virtual void InitializeProjectile(const FProjectileSpawnData& InSpawnParams);
	
protected:
	virtual void BeginPlay() override;
	virtual void LifeSpanExpired() override;
	virtual void SetCollisionEvent();
	bool ShouldIgnoreHit(AActor* OtherActor) const;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> CollisionComp;
    
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UNiagaraComponent> Effect;
	
	UPROPERTY(Transient)
	FProjectileSpawnData SpawnParams;
	
private:
	void ApplyDamageToTarget(AActor* Target, FName HitBoneName);
	void ReturnToPool();

	// 같은 프레임에 충돌/수명 만료가 겹쳐도 풀 반환은 한 번만 수행합니다.
	UPROPERTY(Transient)
	bool bReturnedToPool = false;

	// 풀 재사용 시 원래 충돌 상태를 복원하기 위해 기본값을 보관합니다.
	UPROPERTY(Transient)
	TEnumAsByte<ECollisionEnabled::Type> DefaultCollisionEnabled = ECollisionEnabled::NoCollision;
	
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
