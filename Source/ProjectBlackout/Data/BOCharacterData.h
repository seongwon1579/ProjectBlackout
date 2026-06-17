// ─── 구현 내역 ───────────────────────
//  - 김민영: 병과별 초기 스탯(체력·스태미나·이동/조준/다운 속도)·부여 어빌리티·소모품 슬롯·시작 무기·초기 소지품 데이터 정의
//  - 허혁: 플레이어 스턴 게이지 시스템 필드(초기/최대 게이지·중스턴/브레이크 임계값·감소 지연/속도/틱) 추가
//  - 최승현: 캐릭터 식별 ClassTag·Pawn 클래스·프리뷰 전용 클래스(PreviewClass) 추가
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "BOCharacterData.generated.h"

class ABOFirearm;
class ABOMeleeWeapon;
class UBOConsumableData;
class APawn;

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
	
	/** GetDefaultPawnClassForController 에서 사용할 Pawn Class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Class")
	TSubclassOf<APawn> PawnClass;
	
	/** 캐릭터 선택 프리뷰 전용 클래스 (메시 + 무기 메시 + 전시 애니만 있는 경량 BP).
	 *  미지정 시 PawnClass 로 fallback */
	UPROPERTY(EditAnywhere , BlueprintReadOnly , Category = "Blackout|Class")
	TSubclassOf<AActor> PreviewClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float BaseMaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float BaseMaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float BaseMovementSpeed = 600.f;

	/** 조준 시의 이동 속도 (기본값: 420.f) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float AimMovementSpeed = 420.f;

	/** 다운 상태(기어다니기)에서의 이동 속도 (기본값: 150.f) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stats")
	float DownedMovementSpeed = 150.f;

	/** 전투 시작 시 플레이어에게 부여할 초기 스턴 게이지 값입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stun", meta = (ClampMin = 0.0, ClampMax = 100.0))
	float BaseStunGauge = 0.f;

	/** 플레이어 스턴 게이지의 최대값입니다. 스펙상 0~100 범위로 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stun", meta = (ClampMin = 1.0, ClampMax = 100.0))
	float MaxStunGauge = 100.f;

	/** 이 값 이상부터는 경피격 대신 스턴 몽타주를 재생합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stun", meta = (ClampMin = 0.0, ClampMax = 100.0))
	float HeavyStunThreshold = 50.f;

	/** 이 값 이상부터는 스턴 브레이크(다운 연출)로 전환하고 게이지를 초기화합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stun", meta = (ClampMin = 0.0, ClampMax = 100.0))
	float StunBreakThreshold = 100.f;

	/** 마지막 피격 후 이 시간(초)이 지나면 스턴 게이지 감소를 시작합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stun", meta = (ClampMin = 0.0))
	float StunDecayDelay = 2.0f;

	/** 스턴 게이지 감소 속도입니다. 초당 이 값만큼 감소합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stun", meta = (ClampMin = 0.0))
	float StunDecayPerSecond = 50.0f;

	/** 스턴 게이지 감소를 몇 초 간격으로 적용할지 결정합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Stun", meta = (ClampMin = 0.01))
	float StunDecayTickInterval = 0.1f;

	/** 전투 맵 진입 시 지급되는 블러드 루트 기본 소지량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumables", meta = (ClampMin = 0))
	int32 InitialBloodRoot = 1;

	/** 전투 맵 진입 시 지급되는 굴 혈청 기본 소지량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackout|Consumables", meta = (ClampMin = 0))
	int32 InitialGulSerum = 1;
	
	/** 캐릭터 선택 UI 등에 표시할 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly , Category="Blackout|UI")
	FText DisplayName;
	
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
