#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "BOCharacterData.generated.h"

class ABOFirearm;
class ABOMeleeWeapon;
class UBOConsumableData;

/**
 * 플레이어 병과(Assault / Demolition / Sniper)별 초기 스탯 및 어빌리티 데이터.
 * ABlackoutPlayerState::ApplyBattleTransitionPolicy, ABlackoutLobbyGameMode::PostLogin에서 참조.
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBOCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 이 데이터 에셋이 대응하는 병과 태그 (e.g. Character.Class.Assault) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Class")
	FGameplayTag ClassTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float BaseMaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float BaseMaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float BaseMovementSpeed = 600.f;

	/** 전투 맵 진입 시 지급되는 블러드 루트 기본 소지량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumables", meta = (ClampMin = 0))
	int32 InitialBloodRoot = 1;

	/** 전투 맵 진입 시 지급되는 굴 혈청 기본 소지량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumables", meta = (ClampMin = 0))
	int32 InitialGulSerum = 1;

	/** 캐릭터 선택 UI에 표시할 초상화 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|UI")
	TSoftObjectPtr<UTexture2D> PortraitIcon;

	/** 빙의 시(PossessedBy) ASC에 일괄 부여할 어빌리티 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	/** 소모품 슬롯 순서입니다. 0번은 UseConsumable1, 1번은 UseConsumable2 입력에 연결됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumables")
	TArray<TObjectPtr<UBOConsumableData>> ConsumableSlots;

	/** 시작 주무기 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapons")
	TSubclassOf<ABOFirearm> StartingPrimaryWeapon;

	/** 시작 보조무기 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapons")
	TSubclassOf<ABOFirearm> StartingSecondaryWeapon;

	/** 시작 근접 무기 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Weapons")
	TSubclassOf<ABOMeleeWeapon> StartingMeleeWeapon;
};
