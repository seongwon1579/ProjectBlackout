#include "Animation/Notifies/BOAnimNotify_DodgeChainWindowOpen.h"

#include "Components/SkeletalMeshComponent.h"
#include "GAS/Abilities/Player/BlackoutGA_Dodge.h"

void UBOAnimNotify_DodgeChainWindowOpen::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (UBlackoutGA_Dodge* DodgeAbility = UBlackoutGA_Dodge::GetActiveDodgeAbilityFromActor(MeshComp->GetOwner()))
	{
		DodgeAbility->HandleChainWindowOpened();
	}
}

FString UBOAnimNotify_DodgeChainWindowOpen::GetNotifyName_Implementation() const
{
	return TEXT("DodgeChainOpen");
}
