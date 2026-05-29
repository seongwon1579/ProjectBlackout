// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/EQS/BOEnvQueryTest_IsHigher.h"

#include "Components/CapsuleComponent.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "GameFramework/Character.h"

UBOEnvQueryTest_IsHigher::UBOEnvQueryTest_IsHigher()
{
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();

	SetWorkOnFloatValues(true);
}

void UBOEnvQueryTest_IsHigher::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner)
	{
		return; 
	}

	// 1. 기준점(Querier = AI 자신)의 위치 가져오기
	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(UEnvQueryContext_Querier::StaticClass(), ContextLocations)
		|| ContextLocations.Num() == 0)
	{
		return;
	}

	const FVector QuerierLoc = ContextLocations[0];

	// 캐릭터 발바닥 높이로 환산.
	// 액터 원점은 캡슐 중심이라, EQS 점(바닥 기준)과 비교하려면 캡슐 절반만큼 내려야 함.
	// 캡슐 값을 캐릭터에서 직접 읽으므로 맵이 바뀌어도 매직넘버 없이 일관됨.
	float QuerierFootZ = QuerierLoc.Z;
	if (const ACharacter* OwnerChar = Cast<ACharacter>(QueryOwner))
	{
		QuerierFootZ -= OwnerChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	// 2. 에디터에서 입력한 Min/Max 임계값 바인딩
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	const float MinThreshold = FloatValueMin.GetValue();
	const float MaxThreshold = FloatValueMax.GetValue();

	// 3. 모든 포인트를 순회하며 Z축 높이 차이 계산 (발바닥 기준)
	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		const FVector ItemLoc = GetItemLocation(QueryInstance, It.GetIndex());

		// 발바닥 기준 상대 높이. 평지면 0, 고지대면 양수.
		const float HeightDelta = ItemLoc.Z - QuerierFootZ;
		
		const float HorizDist = FVector::Dist2D(ItemLoc, QuerierLoc);
		UE_LOG(LogTemp, Warning, TEXT("[POINT] Delta=%.1f HorizDist=%.1f"),
			HeightDelta, HorizDist);

		It.SetScore(TestPurpose, FilterType, HeightDelta, MinThreshold, MaxThreshold);
	}
}

FText UBOEnvQueryTest_IsHigher::GetDescriptionTitle() const
{
	return FText::FromString(TEXT("Is Higher Than Querier"));
}

FText UBOEnvQueryTest_IsHigher::GetDescriptionDetails() const
{
	return FText::FromString(TEXT("Filters and scores items by signed Z-height relative to the querier."));
}