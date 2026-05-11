// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BehaviorTree/Decorators/BTD_OutOfChaseDistance.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "Engine/Engine.h"

UBTD_OutOfChaseDistance::UBTD_OutOfChaseDistance()
{
	NodeName = TEXT("OutOfChaseDistance");
	
	bCreateNodeInstance = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTD_OutOfChaseDistance::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	auto BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		auto KeyID = OnTrigger.GetSelectedKeyID();

		BB->RegisterObserver(KeyID, this, FOnBlackboardChangeNotification::CreateUObject(
			this, &UBTD_OutOfChaseDistance::OnBlackboardKeyChanged));

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			TEXT("[BTD_OutOfChaseDistance] ▶ 데코레이터 활성 — BB 옵저버 등록"));
	}
}

void UBTD_OutOfChaseDistance::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	auto BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		BB->UnregisterObserversFrom(this);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			TEXT("[BTD_OutOfChaseDistance] ■ 데코레이터 비활성 — BB 옵저버 해제"));
	}
}

bool UBTD_OutOfChaseDistance::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    auto BB = OwnerComp.GetBlackboardComponent();

	//bool bTrigger = BB->GetValue<UBlackboardKeyType_Bool>(OnTrigger.GetSelectedKeyID());
	bool bTrigger = BB->GetValueAsBool(OnTrigger.SelectedKeyName);
	bool bResult = !bTrigger;

	if (GEngine) GEngine->AddOnScreenDebugMessage(300, 0.15f, FColor::Cyan,
		FString::Printf(TEXT("[BTD_OutOfChaseDistance] ConditionValue — OnTrigger=%s → 결과=%s"),
			bTrigger ? TEXT("TRUE") : TEXT("FALSE"),
			bResult  ? TEXT("PASS") : TEXT("FAIL")));

	return !bTrigger;
}

void UBTD_OutOfChaseDistance::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);
	    
	UBlackboardData* BBAsset = GetBlackboardAsset();
	if (BBAsset)
	{
		OnTrigger.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTD_OutOfChaseDistance::GetStaticDescription() const
{
	return Super::GetStaticDescription();
}

EBlackboardNotificationResult UBTD_OutOfChaseDistance::OnBlackboardKeyChanged(const UBlackboardComponent& BB,
	FBlackboard::FKey KeyID)
{
	UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BB.GetBrainComponent());
	if (BTComp)
	{
		BTComp->RequestExecution(this);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan,
			TEXT("[BTD_OutOfChaseDistance] !! BB 키 변경 감지 → RequestExecution 호출"));
	}

	return EBlackboardNotificationResult::ContinueObserving;
}
