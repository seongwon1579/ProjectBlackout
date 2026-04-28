#include "Animation/Notifies/BOAnimNotify_MeleeCollisionEnable.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

void UBOAnimNotify_MeleeCollisionEnable::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleMeleeCollisionEnabled();
	}
}

FString UBOAnimNotify_MeleeCollisionEnable::GetNotifyName_Implementation() const
{
	return TEXT("MeleeCollisionOn");
}
