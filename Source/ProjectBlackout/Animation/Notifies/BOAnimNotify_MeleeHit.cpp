#include "Animation/Notifies/BOAnimNotify_MeleeHit.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Player/BlackoutGA_MeleePlayer.h"

void UBOAnimNotify_MeleeHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleMeleeHitNotify();
	}
}

FString UBOAnimNotify_MeleeHit::GetNotifyName_Implementation() const
{
	return TEXT("MeleeHit");
}
