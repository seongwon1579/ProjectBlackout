// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutShrewdAIController.h"

#include "Components/StateTreeAIComponent.h"

void ABlackoutShrewdAIController::StartCombat()
{
	Super::StartCombat();
	
	if (HasAuthority() && StateTreeComp)
	{
		StateTreeComp->StartLogic();
	}
}

void ABlackoutShrewdAIController::OnUnPossess()
{
	if (HasAuthority() && StateTreeComp)
	{
		StateTreeComp->StopLogic("UnPossess");
	}
	
	Super::OnUnPossess();
}

void ABlackoutShrewdAIController::HandleAggroTargetChanged(APawn* NewTarget)
{
	CurrentAggroTarget = NewTarget;
}
