#include "Animation/Notifies/BOAnimNotify_MeleeCollisionDisable.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

void UBOAnimNotify_MeleeCollisionDisable::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleMeleeCollisionDisabled();
	}
}

FString UBOAnimNotify_MeleeCollisionDisable::GetNotifyName_Implementation() const
{
	return TEXT("MeleeCollisionOff");
}
