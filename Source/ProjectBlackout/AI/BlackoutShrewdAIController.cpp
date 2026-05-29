// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutShrewdAIController.h"

#include "BOShrewdBoss.h"

void ABlackoutShrewdAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void ABlackoutShrewdAIController::OnUnPossess()
{
	Super::OnUnPossess();
}

void ABlackoutShrewdAIController::HandleAggroTargetChanged(APawn* NewTarget)
{
	//CurrentAggroTarget = NewTarget;
	
	CurrentAggroTarget = NewTarget;
	const FString TargetName = NewTarget ? NewTarget->GetName() : TEXT("None");

	// if (ABOShrewdBoss* Shrewd = Cast<ABOShrewdBoss>(GetPawn()))
	// {
	// 	Shrewd->Multicast_DebugAggroTarget(TargetName);
	// }

}
