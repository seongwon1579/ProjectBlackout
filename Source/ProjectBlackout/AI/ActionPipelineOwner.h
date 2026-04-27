#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ActionPipelineOwner.generated.h"

class UActionPipeline;

UINTERFACE(MinimalAPI)
class UActionPipelineOwner : public UInterface
{
	GENERATED_BODY()
};

class IActionPipelineOwner
{
	GENERATED_BODY()

public:
	virtual UActionPipeline* GetActionPipeline() const = 0;
};
