#include "Characters/BlackoutPlayerMovementComponent.h"

#include "GameFramework/Character.h"

UBlackoutPlayerMovementComponent::UBlackoutPlayerMovementComponent()
{
}

void UBlackoutPlayerMovementComponent::SetSprintRequested(bool bRequested)
{
	bSprintRequested = bRequested;
}

float UBlackoutPlayerMovementComponent::GetMaxSpeed() const
{
	const float BaseMaxSpeed = Super::GetMaxSpeed();
	if (!bSprintRequested)
	{
		return BaseMaxSpeed;
	}

	return BaseMaxSpeed * SprintSpeedMultiplier;
}

void UBlackoutPlayerMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bSprintRequested = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* UBlackoutPlayerMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UBlackoutPlayerMovementComponent* MutableThis = const_cast<UBlackoutPlayerMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_BlackoutPlayer(*this);
	}

	return ClientPredictionData;
}

void FSavedMove_BlackoutPlayer::Clear()
{
	Super::Clear();
	bSavedSprintRequested = false;
}

uint8 FSavedMove_BlackoutPlayer::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedSprintRequested)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

bool FSavedMove_BlackoutPlayer::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	const FSavedMove_BlackoutPlayer* NewBlackoutMove = static_cast<const FSavedMove_BlackoutPlayer*>(NewMove.Get());
	if (bSavedSprintRequested != NewBlackoutMove->bSavedSprintRequested)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void FSavedMove_BlackoutPlayer::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	if (const UBlackoutPlayerMovementComponent* MoveComp =
		Cast<UBlackoutPlayerMovementComponent>(Character ? Character->GetCharacterMovement() : nullptr))
	{
		bSavedSprintRequested = MoveComp->IsSprintRequested();
	}
}

void FSavedMove_BlackoutPlayer::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	if (UBlackoutPlayerMovementComponent* MoveComp =
		Cast<UBlackoutPlayerMovementComponent>(Character ? Character->GetCharacterMovement() : nullptr))
	{
		MoveComp->SetSprintRequested(bSavedSprintRequested);
	}
}

FNetworkPredictionData_Client_BlackoutPlayer::FNetworkPredictionData_Client_BlackoutPlayer(
	const UCharacterMovementComponent& ClientMovement)
	: FNetworkPredictionData_Client_Character(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_BlackoutPlayer::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_BlackoutPlayer());
}
