// ─── 구현 내역 ───────────────────────
//  - 조성원: Ravager 메인 보스 — 데이터 기반 패턴/스탯 + 체력 비율 페이즈 판정 + 어빌리티별 콜리전 상태 전환 + 추격 범위 제공
//  - 최승현: 보스 HP 정원 스케일링(1인/4인)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutBossCharacter.h"
#include "Enum/BOBossPhase.h"
#include "BORavagerBoss.generated.h"


class UBORavagerPatternData;
class UBORavagerStatData;

UCLASS()
class PROJECTBLACKOUT_API ABORavagerBoss : public ABlackoutBossCharacter
{
	GENERATED_BODY()

public:
	UFUNCTION()
	UBORavagerPatternData* GetPatternData(FGameplayTag AbilityTag) const;
	
	void SetCollisionState(bool bIgnore);
	
	virtual void SetData() override;
	
	virtual FBossChaseRanges GetChaseRanges(const FGameplayTag& PatternTag) const override;

protected:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCollisionState(bool bIgnore);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Blackout|Collision")
	void BP_OnCollisionStateChanged(bool bEnable);
	
	virtual void OnDeath() override;
	
	
	virtual void OnDamageReceived(const FOnAttributeChangeData& Data);
	
	virtual FText GetBossDisplayName() const override;
	
	EBOBossPhase DetermineTargetPhase(float HealthRatio);
	
	APawn* ResolveInstigatorPawn(AActor* SourceActor) const;

	UPROPERTY(EditAnywhere, Category = "Blackout|Data", meta = (Categories = "Ability"))
	TMap<FGameplayTag, TObjectPtr<UBORavagerPatternData>> BossPatternData;

	UPROPERTY(EditAnywhere, Category = "Blackout|Data")
	TObjectPtr<UBORavagerStatData> BossStatData;
};
