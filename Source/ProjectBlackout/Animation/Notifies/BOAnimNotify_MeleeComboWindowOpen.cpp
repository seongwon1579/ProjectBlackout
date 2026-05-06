#include "Animation/Notifies/BOAnimNotify_MeleeComboWindowOpen.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

void UBOAnimNotify_MeleeComboWindowOpen::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleComboWindowOpened();
	}
}

FString UBOAnimNotify_MeleeComboWindowOpen::GetNotifyName_Implementation() const
{
	return TEXT("ComboWindowOpen");
}
