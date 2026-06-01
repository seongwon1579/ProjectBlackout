// Fill out your copyright notice in the Description page of Project Settings.


#include "BOCharacterRoster.h"
#include "BOCharacterData.h"

UBOCharacterData* UBOCharacterRoster::FindByClassTag(
	FGameplayTag ClassTag) const
{
	
	for (const TObjectPtr<UBOCharacterData>& Data : Characters)
	{
		if (Data && Data->ClassTag.MatchesTagExact(ClassTag))
		{
			return Data;
		}
	}
	
	return nullptr;
}

TSubclassOf<APawn> UBOCharacterRoster::FindPawnClassByTag(
	FGameplayTag ClassTag) const
{
	if (const UBOCharacterData* Data = FindByClassTag(ClassTag))
	{
		return Data->PawnClass;
	}
	return nullptr;
}
