#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Data/BlackoutWeaponStat.h"
#include "BOWeaponBase.generated.h"

class USkeletalMeshComponent;
class ABlackoutCharacterBase;

UCLASS(Abstract)
class PROJECTBLACKOUT_API ABOWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ABOWeaponBase();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	ABlackoutCharacterBase* GetOwningCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	FGameplayTag GetWeaponTag() const { return WeaponTag; }

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	float GetBaseDamage() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	int32 GetMaxReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Blackout|Combat")
	void AttachToOwner(FName SocketName);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|Combat")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Combat")
	FGameplayTag WeaponTag;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|Combat")
	FBlackoutWeaponStat CachedStats;
};
