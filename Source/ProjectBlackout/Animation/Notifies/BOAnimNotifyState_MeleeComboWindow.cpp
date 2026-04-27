#include "Animation/Notifies/BOAnimNotifyState_MeleeComboWindow.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

void UBOAnimNotifyState_MeleeComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleComboWindowOpened();
	}
}

void UBOAnimNotifyState_MeleeComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleComboWindowClosed();
	}
}

FString UBOAnimNotifyState_MeleeComboWindow::GetNotifyName_Implementation() const
{
	return TEXT("MeleeComboWindow");
}
