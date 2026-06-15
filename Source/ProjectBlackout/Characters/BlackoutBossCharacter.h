#pragma once

#include "CoreMinimal.h"
#include "BOBossChaseRanges.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "GameplayEffectTypes.h"
#include "MotionWarpingComponent.h"

#include "BlackoutBossCharacter.generated.h"

class UBOBossData;
class UMotionWarpingComponent;

DECLARE_MULTICAST_DELEGATE(FBlackoutBossDefeatedSignature);

UCLASS(Abstract)
class PROJECTBLACKOUT_API ABlackoutBossCharacter : public ABlackoutEnemyCharacter
{
	GENERATED_BODY()

public:
	ABlackoutBossCharacter();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blackout|MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	/** 서버에서 보스 사망 시 1회 발행. GameMode가 바인딩해 매치 분기 처리. */
	FBlackoutBossDefeatedSignature OnDefeated;
	
	virtual void SetData() {}
	
	virtual FBossChaseRanges GetChaseRanges(const FGameplayTag& PatternTag) const
	{
		return FBossChaseRanges();
	}

protected:
	virtual void BeginPlay() override;

	virtual void OnDeath() override;
	
	virtual void OnDamageReceived(const FOnAttributeChangeData& Data) {}
 
	virtual FText GetBossDisplayName() const { return FText::FromString(TEXT("")); }
	
	void TryBindToHUD();

private:
	/** OnDefeated broadcast 단일 발화 가드 — 죽음 후 추가 damage/재호출에도 1회만 브로드캐스트. */
	bool bDefeatedBroadcastSent = false;
	
	/** TryBindToHuD 재시도 카운터 **/
	int32 HUDBindRetryCount = 0;
	static constexpr  int32 HUDBindMaxRetries =120;
	
	
};
