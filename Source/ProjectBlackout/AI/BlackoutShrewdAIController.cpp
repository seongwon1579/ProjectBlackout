// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutShrewdAIController.h"

#include "BOShrewdBoss.h"

void ABlackoutShrewdAIController::HandleAggroTargetChanged(APawn* NewTarget)
{
	CurrentAggroTarget = NewTarget;
}
