// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include  "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "BOCharacterRoster.generated.h"


class UBOCharacterData;
class APawn;

/**
 *  캐릭터 선택 UI / GetDefaultPawnClassForController 에서 참조
 * 
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBOCharacterRoster : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	/** 선택 UI 노출 순서. 인덱스 0 이 기본 */
	UPROPERTY(EditAnywhere , BlueprintReadOnly, Category = "Blackout|Category")
	TArray<TObjectPtr<UBOCharacterData>> Characters;
	
	/** ClassTag로 캐릭터 데이터 조회 없으면 nullptr */
	UBOCharacterData* FindByClassTag(FGameplayTag ClassTag) const;
	
	/** ClassTag로 Pawn 클래스 조회. 없거나 PawnClass 미설정 이면 nullptr */
	TSubclassOf<APawn> FindPawnClassByTag(FGameplayTag ClassTag) const;
	
};
