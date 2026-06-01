#pragma once

#include "CoreMinimal.h"
#include "Data/BlackoutWeaponStat.h"
#include "BlackoutClassSelectTypes.generated.h"

class UBOCharacterData;

/**   
 *   캐릭터 선택 UI 페이지 표시 데이터
 *   Controller 가 인덱스 변경 시 이 구조체로 broadcast. Widget 은 BOCharacterData 필드를
 *   직접 바인딩하고, 무기 stat 은 미리 lookup 된 PrimaryStat/SecondaryStat 사용
 */

USTRUCT(BlueprintType)
struct PROJECTBLACKOUT_API FBlackoutClassSelectDisplayData
{
	GENERATED_BODY()
	
	/** 캐릭터 데이터 원본 - DisplayName/Portrait/MaxHealth/MaxStamina 등 */
	UPROPERTY(BlueprintReadOnly,Category="Blackout|ClassSelect")
	TObjectPtr<const UBOCharacterData> CharacterData  =nullptr;
	
	/** 주무기 stat - ABOFirearm CDO -> DataTable lookup 결과를 Controller 캐싱 */
	UPROPERTY(BlueprintReadOnly , Category="Blackout|ClassSelect")
	FBlackoutFirearmStat PrimaryStat;
	
	UPROPERTY(BlueprintReadOnly , Category="Blackout|ClassSelect")
	FBlackoutFirearmStat SecondaryStat;
};
