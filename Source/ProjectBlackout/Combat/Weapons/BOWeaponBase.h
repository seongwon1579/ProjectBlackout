// ─── 구현 내역 ───────────────────────
//  - 김민영: 무기 베이스 계층(소켓 부착, 스탯 데이터테이블 초기화, GCN 태그·크로스헤어 노출, 왼손 IK 타겟, 장착 상태 연동) 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/BlackoutWeaponStat.h"
#include "GameplayTagContainer.h"
#include "BOWeaponBase.generated.h"

class USkeletalMeshComponent;
class UTexture2D;
class ABlackoutCharacterBase;

UCLASS(Abstract)
class PROJECTBLACKOUT_API ABOWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ABOWeaponBase();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABlackoutCharacterBase* GetOwningCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Weapon")
	UTexture2D* GetWeaponIcon() const { return WeaponIcon; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|UI")
	int32 GetCrosshairType() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetBaseDamage() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Cue")
	FBlackoutWeaponCueSet GetWeaponCueSet() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Cue")
	FGameplayTag GetFireCueTag() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Cue")
	FGameplayTag GetTrailCueTag() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Cue")
	FGameplayTag ResolveImpactCueTag(FGameplayTag SurfaceTag) const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	FName GetEquippedSocketName() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Combat")
	FName GetHolsterSocketName() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Weapon")
	virtual bool InitializeStatsFromDataTable();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	bool AttachToOwner(FName SocketName);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	virtual void OnEquipStateChanged(bool bEquipped);

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	bool HasLeftHandIKTarget() const;

	UFUNCTION(BlueprintPure, Category = "Blackout|Animation")
	FTransform GetLeftHandIKTransform() const;

protected:
	virtual void BeginPlay() override;
	void ApplyCommonStats(const FBlackoutWeaponStat& WeaponStats);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|UI")
	TObjectPtr<UTexture2D> WeaponIcon;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Combat")
	FBlackoutWeaponStat CachedStats;

	/** 왼손 IK가 고정될 무기 메시 소켓 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Animation")
	FName LeftHandIKSocketName = NAME_None;
};
