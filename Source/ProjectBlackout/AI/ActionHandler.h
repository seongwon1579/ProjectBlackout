// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ActionHandler.generated.h"

/**
 * 
 */
struct FActionData;

UINTERFACE()
class PROJECTBLACKOUT_API UActionHandler : public UInterface
{
	GENERATED_BODY()
};

class IActionHandler
{
	GENERATED_BODY()

public:
	virtual bool CanExecute(const FActionData& Data) const = 0;
	virtual void Execute(FActionData& Data) = 0;
	virtual void Undo(const FActionData& Data) {}
	virtual int32 GetPriority() const = 0;
};
