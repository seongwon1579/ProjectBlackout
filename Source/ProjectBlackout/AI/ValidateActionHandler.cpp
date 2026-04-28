// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/ValidateActionHandler.h"
#include "AI/FActionData.h"
#include "AI/BOAICalcHelper.h"

bool UValidateActionHandler::CanExecute(const FActionData& Data) const
{
	if (!Data.Instigator || Data.Instigator->IsPendingKillPending())
	{
		return false;
	}

	if (Data.Target && Data.Target->IsPendingKillPending())
	{
		return false;
	}

	if (Data.Target && Data.Range > 0.f)
	{
		if (!UBOAICalcHelper::IsWithinRange(Data.Instigator, Data.Target, Data.Range))
		{
			return false;
		}
	}

	return true;
}

void UValidateActionHandler::Execute(FActionData& Data)
{
	Data.SpawnTransform = Data.Instigator->GetActorTransform();
}
