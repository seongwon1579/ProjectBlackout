#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutBossCharacter.h"
#include "Enum/BOBossPhase.h"
#include "BORavagerBoss.generated.h"


class UBORavagerPatternData;
class UBORavagerStatData;

UCLASS()
class PROJECTBLACKOUT_API ABORavagerBoss : public ABlackoutBossCharacter
{
	GENERATED_BODY()

public:
	UFUNCTION()
	UBORavagerPatternData* GetPatternData(FGameplayTag AbilityTag) const;
	
	void SetCollisionState(bool bIgnore);
	
	virtual void SetData() override;
	
	virtual FBossChaseRanges GetChaseRanges(const FGameplayTag& PatternTag) const override;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCollisionState(bool bIgnore);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Blackout|Collision")
	void BP_OnCollisionStateChanged(bool bEnable);
	
	virtual void OnDeath() override;
	
	
	virtual void OnDamageReceived(const FOnAttributeChangeData& Data);
	
	virtual FText GetBossDisplayName() const override;
	
	EBOBossPhase DetermineTargetPhase(float HealthRatio);
	
	APawn* ResolveInstigatorPawn(AActor* SourceActor) const;

	UPROPERTY(EditAnywhere, Category = "Blackout|Data", meta = (Categories = "Ability"))
	TMap<FGameplayTag, TObjectPtr<UBORavagerPatternData>> BossPatternData;

	UPROPERTY(EditAnywhere, Category = "Blackout|Data")
	TObjectPtr<UBORavagerStatData> BossStatData;
};
