// ─── 구현 내역 ───────────────────────
//  - 조성원: Shrewd 중간 보스 생성 — 텔레포트 위치 반환 + 데이터 세팅 + 사망 시 AI 종료
//  - 최승현: Shrewd 비행 보스 어그로/카이팅/발사 StateTree 및 HP 정원 스케일링
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Characters/BlackoutBossCharacter.h"
#include "BOShrewdBoss.generated.h"

class UUBOShrewdData;
class UBlackoutAggroComponent;

UCLASS()
class PROJECTBLACKOUT_API ABOShrewdBoss : public ABlackoutBossCharacter
{
	GENERATED_BODY()

public:
	ABOShrewdBoss();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_DebugAggroTarget(const FString& TargetName);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Blackout")
	void SetBowVisible(bool bVisible);
	
	bool GetRandomTeleportTransform(FTransform& OutTransform);
		
protected:
	
	UPROPERTY(EditAnywhere, Category = "Blackout")
	TArray<TObjectPtr<AActor>> TeleportPoints;
	
	UPROPERTY()
	TObjectPtr<AActor> LastTeleportPoint;
	
	UPROPERTY(EditAnywhere, Category = "Blackout")
	TObjectPtr<UUBOShrewdData> ShrewdData;
	
	UPROPERTY(VisibleAnywhere , Category = "Blackout")
	TObjectPtr<UBlackoutAggroComponent> AggroComponent;
	
	virtual void SetData() override;

	virtual void OnDamageReceived(const FOnAttributeChangeData& Data) override;
 
	virtual FText GetBossDisplayName() const override;
	
	APawn* ResolveInstigatorPawn(AActor* SourceActor) const;
	
	virtual void OnDeath() override;
	
};
