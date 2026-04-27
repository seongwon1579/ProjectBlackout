#include "AI/ActionPipeline.h"

#include "ActionHandler.h"
#include "ValidateActionHandler.h"

DEFINE_LOG_CATEGORY_STATIC(LogActionPipeline, Log, All);

// 새 전처리 단계 추가 시 RegisterHandler 한 줄 + 핸들러 클래스만 추가.
void UActionPipeline::Initialize()
{
	RegisterHandler(NewObject<UValidateActionHandler>(this));
}

void UActionPipeline::RegisterHandler(UObject* Handler)
{
	if (!Handler || !Handler->GetClass()->ImplementsInterface(UActionHandler::StaticClass()))
	{
		UE_LOG(LogActionPipeline, Error, TEXT("RegisterHandler: IActionHandler를 구현하지 않은 객체 [%s]"),
			*GetNameSafe(Handler));
		return;
	}
	HandlerList.Add(Handler);
}

bool UActionPipeline::Execute(FActionData& Data)
{
	TArray<IActionHandler*> Sorted = GetSortedHandlers();

#if !UE_BUILD_SHIPPING
	UE_LOG(LogActionPipeline, Log, TEXT("=== Pre-GA Pipeline Start [%s] ==="), *GetNameSafe(Data.Instigator));
#endif

	TArray<IActionHandler*> Executed;

	for (IActionHandler* Handler : Sorted)
	{
		UObject* HandlerObj = Cast<UObject>(Handler);

		if (!Handler->CanExecute(Data))
		{
			Data.bChainFailed = true;

#if !UE_BUILD_SHIPPING
			UE_LOG(LogActionPipeline, Log, TEXT("  [FAIL] %s"), *GetNameSafe(HandlerObj));
#endif
			RollbackExecutedHandlers(Executed, Data);
			OnActionStepFailed.Broadcast(HandlerObj, Data);
			return false;
		}

		Handler->Execute(Data);
		Executed.Add(Handler);

#if !UE_BUILD_SHIPPING
		UE_LOG(LogActionPipeline, Log, TEXT("  [ OK ] %s"), *GetNameSafe(HandlerObj));
#endif
		OnActionStepExecuted.Broadcast(HandlerObj, Data);
	}

#if !UE_BUILD_SHIPPING
	UE_LOG(LogActionPipeline, Log, TEXT("=== Pre-GA Pipeline Complete → Target: %s ==="),
		*GetNameSafe(Data.Target));
#endif

	OnActionChainCompleted.Broadcast(Data);
	return true;
}

// ─────────────────────────────────────────────────────────────────────────────

TArray<IActionHandler*> UActionPipeline::GetSortedHandlers() const
{
	TArray<IActionHandler*> Result;
	for (const TObjectPtr<UObject>& Obj : HandlerList)
	{
		if (IActionHandler* Handler = Cast<IActionHandler>(Obj.Get()))
		{
			Result.Add(Handler);
		}
	}
	Result.Sort([](const IActionHandler& A, const IActionHandler& B)
	{
		return A.GetPriority() < B.GetPriority();
	});
	return Result;
}

void UActionPipeline::RollbackExecutedHandlers(
	const TArray<IActionHandler*>& Executed, const FActionData& Data)
{
	for (int32 i = Executed.Num() - 1; i >= 0; --i)
	{
		Executed[i]->Undo(Data);
	}
}
