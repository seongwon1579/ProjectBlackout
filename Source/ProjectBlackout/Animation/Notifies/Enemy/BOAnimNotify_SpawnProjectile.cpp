// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Enemy/BOAnimNotify_SpawnProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "BlackoutGameplayTags.h"
#include "Abilities/GameplayAbilityTypes.h"


void UBOAnimNotify_SpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (!EventTag.IsValid()) return;
	
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		EventTag,
		EventData
	);
	}
}

FString UBOAnimNotify_SpawnProjectile::GetNotifyName_Implementation() const
{
	return EventTag.IsValid() 
	? FString::Printf(TEXT("SpawnProjectile [%s]"), *EventTag.ToString())
	: TEXT("SpawnProjectile");
}
