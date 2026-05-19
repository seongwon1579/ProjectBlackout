#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UI/BlackoutPartyTypes.h"
#include "BlackoutPartyRosterWidgetController.generated.h"

class ABlackoutGameState;
class ABlackoutPlayerCharacter;
class ABlackoutPlayerState;
class APlayerController;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBlackoutPartyRosterRebuiltSignature,
	const TArray<FBlackoutPartyMemberStatusData>&, MemberStatusList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FBlackoutPartyMemberStatusChangedSignature,
	const FBlackoutPartyMemberStatusData&, MemberStatusData);

struct FBlackoutPartyMemberBinding
{
	TWeakObjectPtr<ABlackoutPlayerState> PlayerState;
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<ABlackoutPlayerCharacter> PlayerCharacter;
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;
	FDelegateHandle DownedStateTagChangedHandle;
	FDelegateHandle BeingRevivedStateTagChangedHandle;
};

UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBlackoutPartyRosterWidgetController : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	bool Initialize(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void BindCallbacksToDependencies();

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void BroadcastInitialRoster();

	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Party")
	void RefreshRoster();

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Party")
	FBlackoutPartyRosterRebuiltSignature OnRosterRebuilt;

	UPROPERTY(BlueprintAssignable, Category = "Blackout|HUD|Party")
	FBlackoutPartyMemberStatusChangedSignature OnMemberStatusChanged;

private:
	bool ResolveDependencies(APlayerController* InPlayerController);
	void BindMember(ABlackoutPlayerState* MemberPlayerState);
	void UnbindMember(ABlackoutPlayerState* MemberPlayerState);
	void UnbindAllMembers();
	void RefreshMemberCharacterBinding(FBlackoutPartyMemberBinding& Binding);
	void BroadcastMemberStatus(ABlackoutPlayerState* MemberPlayerState);
	FBlackoutPartyMemberStatusData BuildStatusData(ABlackoutPlayerState* MemberPlayerState) const;
	ABlackoutPlayerCharacter* ResolvePlayerCharacter(ABlackoutPlayerState* MemberPlayerState) const;
	UAbilitySystemComponent* ResolveAbilitySystemComponent(ABlackoutPlayerState* MemberPlayerState) const;

	UFUNCTION()
	void HandlePlayerArrayChanged();

	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<ABlackoutGameState> GameState;
	TWeakObjectPtr<ABlackoutPlayerState> LocalPlayerState;
	TMap<ABlackoutPlayerState*, FBlackoutPartyMemberBinding> MemberBindings;
	bool bGameStateCallbacksBound = false;
};
