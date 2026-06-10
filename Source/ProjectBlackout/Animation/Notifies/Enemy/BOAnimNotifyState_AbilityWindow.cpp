// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/Enemy/BOAnimNotifyState_AbilityWindow.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"

void UBOAnimNotifyState_AbilityWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                   float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	if (!EventStart.IsValid() || !EventEnd.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("GamePlayTag is not set"))
		return;
	}
	
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;
	
	FGameplayEventData EventData;
	EventData.EventMagnitude = TotalDuration;
	EventData.Instigator     = Owner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		EventStart,
		EventData);
}

void UBOAnimNotifyState_AbilityWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
 
	if (!EventStart.IsValid() || !EventEnd.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("GamePlayTag is not set"))
		return;
	}
	
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		Owner,
		EventEnd,
		FGameplayEventData());
	
}
