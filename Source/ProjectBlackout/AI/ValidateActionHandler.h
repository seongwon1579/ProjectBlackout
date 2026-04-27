
#pragma once

#include "CoreMinimal.h"
#include "AI/ActionHandler.h"

#include "ValidateActionHandler.generated.h"

UCLASS()
class PROJECTBLACKOUT_API UValidateActionHandler : public UObject, public IActionHandler
{
	GENERATED_BODY()

public:
	virtual bool CanExecute(const FActionData& Data) const override;
	virtual void Execute(FActionData& Data) override;
	virtual int32 GetPriority() const override { return 0; }
};