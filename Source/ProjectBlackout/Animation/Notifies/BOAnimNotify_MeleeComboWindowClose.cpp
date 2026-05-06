#include "Animation/Notifies/BOAnimNotify_MeleeComboWindowClose.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

void UBOAnimNotify_MeleeComboWindowClose::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleComboWindowClosed();
	}
}

FString UBOAnimNotify_MeleeComboWindowClose::GetNotifyName_Implementation() const
{
	return TEXT("ComboWindowClose");
}
