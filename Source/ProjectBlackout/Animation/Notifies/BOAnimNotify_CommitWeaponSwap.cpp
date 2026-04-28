#include "Animation/Notifies/BOAnimNotify_CommitWeaponSwap.h"

#include "Characters/BlackoutPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UBOAnimNotify_CommitWeaponSwap::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(MeshComp->GetOwner()))
	{
		PlayerCharacter->CommitPendingWeaponSwap();
	}
}

FString UBOAnimNotify_CommitWeaponSwap::GetNotifyName_Implementation() const
{
	return TEXT("CommitWeaponSwap");
}
