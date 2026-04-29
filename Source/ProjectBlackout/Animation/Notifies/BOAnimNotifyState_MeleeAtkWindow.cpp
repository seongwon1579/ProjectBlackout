// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/BOAnimNotifyState_MeleeAtkWindow.h"

#include "Player/BlackoutGA_MeleePlayer.h"

void UBOAnimNotifyState_MeleeAtkWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                    float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp) return;
	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleMeleeAttackWindowBegin();
	}
	
}

void UBOAnimNotifyState_MeleeAtkWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	
	if (!MeshComp)
	{
		return;
	}

	// 활성 근접 어빌리티에 공격창 갱신을 전달
	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleMeleeAttackWindowTick();
	}
	
}

void UBOAnimNotifyState_MeleeAtkWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                  const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (!MeshComp)
	{
		return;
	}

	// 활성 근접 어빌리티에 공격창 종료를 전달
	if (UBlackoutGA_MeleePlayer* MeleeAbility = UBlackoutGA_MeleePlayer::GetActiveMeleeAbilityFromActor(MeshComp->GetOwner()))
	{
		MeleeAbility->HandleMeleeAttackWindowEnd();
	}
	
}

FString UBOAnimNotifyState_MeleeAtkWindow::GetNotifyName_Implementation() const
{
	return Super::GetNotifyName_Implementation();
}
