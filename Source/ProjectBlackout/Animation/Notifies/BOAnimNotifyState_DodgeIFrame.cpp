// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/BOAnimNotifyState_DodgeIFrame.h"

#include "AbilitySystemInterface.h"
#include "Core/BlackoutLog.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"

namespace
{
	UBlackoutAbilitySystemComponent* ResolveBlackoutAbilitySystemComponent(USkeletalMeshComponent* MeshComp)
	{
		if (!MeshComp)
		{
			return nullptr;
		}

		AActor* OwnerActor = MeshComp->GetOwner();
		const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerActor);
		if (!AbilitySystemInterface)
		{
			return nullptr;
		}

		return Cast<UBlackoutAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
	}
}

void UBOAnimNotifyState_DodgeIFrame::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = ResolveBlackoutAbilitySystemComponent(MeshComp))
	{
		AbilitySystemComponent->AddLooseGameplayTag(BlackoutGameplayTags::State_Invulnerable);
		BO_LOG_GAS(Log, "DodgeIFrame begin: Owner=%s", *GetNameSafe(OwnerActor));
	}
}

void UBOAnimNotifyState_DodgeIFrame::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	if (UBlackoutAbilitySystemComponent* AbilitySystemComponent = ResolveBlackoutAbilitySystemComponent(MeshComp))
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Invulnerable);
		BO_LOG_GAS(Log, "DodgeIFrame end: Owner=%s", *GetNameSafe(OwnerActor));
	}
}

FString UBOAnimNotifyState_DodgeIFrame::GetNotifyName_Implementation() const
{
	return TEXT("BO Dodge IFrame");
}
