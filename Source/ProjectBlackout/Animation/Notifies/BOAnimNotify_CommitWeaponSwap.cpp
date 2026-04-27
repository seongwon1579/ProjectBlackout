#include "Animation/Notifies/BOAnimNotify_CommitWeaponSwap.h"

#include "Characters/BlackoutPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UBOAnimNotify_CommitWeaponSwap::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

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
