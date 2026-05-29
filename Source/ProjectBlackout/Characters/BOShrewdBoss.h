#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutBossCharacter.h"
#include "BOShrewdBoss.generated.h"

class UUBOShrewdData;

UCLASS()
class PROJECTBLACKOUT_API ABOShrewdBoss : public ABlackoutBossCharacter
{
	GENERATED_BODY()

public:
	ABOShrewdBoss();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DebugAggroTarget(const FString& TargetName);
	
	UPROPERTY(EditAnywhere, Category = "Blackout")
	TObjectPtr<AActor> TelePortPtr;
		
protected:
	UPROPERTY(EditAnywhere, Category = "Blackout")
	TObjectPtr<UUBOShrewdData> ShrewdData;
	
	virtual void SetData() override;

	virtual void OnDamageReceived(const FOnAttributeChangeData& Data) override;
 
	virtual FText GetBossDisplayName() const override;
	
	APawn* ResolveInstigatorPawn(AActor* SourceActor) const;
	
};
