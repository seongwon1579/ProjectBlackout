#include "Animation/Notifies/BOAnimNotify_LocalCameraShake.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

void UBOAnimNotify_LocalCameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!ShakeClass || !MeshComp)
	{
		return;
	}

	// 본인이 조종 중인 폰에서만 — 다른 플레이어의 몽타주 재생에 내 화면이 흔들리면 안 됨.
	const APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (!OwnerPawn || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->StartCameraShake(ShakeClass, Scale);
		}
	}
}

FString UBOAnimNotify_LocalCameraShake::GetNotifyName_Implementation() const
{
	return TEXT("LocalCameraShake");
}
