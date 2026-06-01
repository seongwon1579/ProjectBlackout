// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Abilities/Boss/Shrewd/BlackoutGA_Shrewd_TeleportByEQS.h"

#include "EnvironmentQuery/EnvQueryManager.h"

void UBlackoutGA_Shrewd_TeleportByEQS::StartResolveDestination()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar || !TeleportQuery)
	{
		FinishPrepare(false);
		return;
	}
	
	FEnvQueryRequest Request(TeleportQuery, Avatar);
	Request.Execute(EEnvQueryRunMode::SingleResult, this,
					&UBlackoutGA_Shrewd_TeleportByEQS::OnEQSFinished);
}



void UBlackoutGA_Shrewd_TeleportByEQS::OnEQSFinished(TSharedPtr<FEnvQueryResult> Result)
{
	if (!Result.IsValid() || !Result->IsSuccessful() || Result->Items.Num() == 0)
	{
		FinishPrepare(false);
		return;
	}
	
	CachedDestination = Result->GetItemAsLocation(0);
	CachedTeleportRotation = GetAvatarActorFromActorInfo()->GetActorRotation();
	
	FinishPrepare(true);
}
