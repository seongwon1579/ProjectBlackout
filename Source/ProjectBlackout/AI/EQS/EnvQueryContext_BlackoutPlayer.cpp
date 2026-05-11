// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/EnvQueryContext_BlackoutPlayer.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Kismet/GameplayStatics.h"

void UEnvQueryContext_BlackoutPlayer::ProvideContext(
	FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// TODO: 멀티 검증 후 Aggro 연결
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(QueryInstance.Owner.Get(), 0);
	if (!PlayerPawn)
	{
		return;
	}
	UEnvQueryItemType_Actor::SetContextHelper(ContextData,PlayerPawn);
}
