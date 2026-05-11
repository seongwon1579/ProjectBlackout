#pragma once

#include "CoreMinimal.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "BOFirearm.generated.h"

class UNiagaraComponent;
class ABOProjectile;
class UAnimationAsset;

UCLASS()
class PROJECTBLACKOUT_API ABOFirearm : public ABOWeaponBase
{
	GENERATED_BODY()
	
public:
	ABOFirearm();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FHitResult Fire(const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABOProjectile* SpawnProjectile(const FVector& Direction, const FGameplayEffectSpecHandle& DamageSpecHandle);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FTransform GetMuzzleTransform() const;

	virtual bool InitializeStatsFromDataTable() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetFireRate() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool IsAutomatic() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool UsesHitscan() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	bool UsesTwoHandedAnimation() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	FGameplayTag GetFireAnimTag() const { return FireAnimTag; }

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	FGameplayTag GetReloadAnimTag() const { return ReloadAnimTag; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMaxReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetSplashRadius() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> GetProjectileClass() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetProjectileLaunchSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetProjectileGravityScale() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetProjectileCollisionRadius() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetProjectileImpactFuseArmDistance() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetBaseSpreadDegrees() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetMaxSpreadDegrees() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetSpreadPerShot() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetSpreadRecoveryRate() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetVerticalRecoilMin() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetVerticalRecoilMax() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetHorizontalRecoilRange() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetMaxRecoilPitchDegrees() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetRecoilRecoveryFraction() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayWeaponFireAnimation();

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayWeaponFireAnimation();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool StopWeaponFireAnimation();

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_StopWeaponFireAnimation();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool PlayWeaponReloadAnimation();

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_PlayWeaponReloadAnimation();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Animation")
	bool StopWeaponReloadAnimation();

	UFUNCTION(NetMulticast, Reliable, Category = "Blackout|Animation")
	void Multicast_StopWeaponReloadAnimation();

protected:
	void ApplyFirearmStats(const FBlackoutFirearmStat& FirearmStats);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<UNiagaraComponent> MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Weapon", meta = (RowType = "/Script/ProjectBlackout.BlackoutFirearmStat"))
	FDataTableRowHandle FirearmStatsRow;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Weapon")
	FBlackoutFirearmStat CachedFirearmStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FName MuzzleSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	TSubclassOf<ABOProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	bool bUseHitscan = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	bool bUseTwoHandedAnimation = true;

	/** 현재 무기가 사용할 사격 애니메이션 프로필 태그입니다. 실제 몽타주는 캐릭터가 소유합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	FGameplayTag FireAnimTag;

	/** 현재 무기가 사용할 재장전 애니메이션 프로필 태그입니다. 실제 몽타주는 캐릭터가 소유합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation") 
	FGameplayTag ReloadAnimTag;

	/** Huntmaster 같은 단발식 무기가 발사 직후 무기 메시에 직접 재생할 애니메이션입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimationAsset> WeaponFireAnimation;

	/** 무기 메시에 직접 재생할 재장전 애니메이션입니다. 캐릭터 상체 몽타주와 별도로 동작합니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	TObjectPtr<UAnimationAsset> WeaponReloadAnimation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug")
	bool bDrawDebugHitscanRay = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug", meta = (EditCondition = "bDrawDebugHitscanRay", ClampMin = 0.f))
	float DebugHitscanRayDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Debug", meta = (EditCondition = "bDrawDebugHitscanRay", ClampMin = 0.f))
	float DebugHitscanRayThickness = 1.5f;
};
