#include "BlackoutPlayerState.h"
#include "BlackoutAbilitySystemComponent.h"
#include "BlackoutAmmoAttributeSet.h"
#include "Attributes/BlackoutBaseAttributeSet.h"
#include "Data/BOCharacterData.h"
#include "Data/BOConsumableData.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "BlackoutLog.h"
#include "BlackoutPlayerAttributeSet.h"

ABlackoutPlayerState::ABlackoutPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<UBlackoutAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	BaseAttributeSet = CreateDefaultSubobject<UBlackoutBaseAttributeSet>(TEXT("BaseAttributeSet"));
	PlayerAttributeSet = CreateDefaultSubobject<UBlackoutPlayerAttributeSet>(TEXT("PlayerAttributeSet"));
	AmmoAttributeSet = CreateDefaultSubobject<UBlackoutAmmoAttributeSet>(TEXT("AmmoAttributeSet"));

	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* ABlackoutPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABlackoutPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutPlayerState, SelectedClassTag);
	DOREPLIFETIME(ABlackoutPlayerState, BloodRootCount);
	DOREPLIFETIME(ABlackoutPlayerState, GulSerumCount);
	DOREPLIFETIME(ABlackoutPlayerState, bIsReady);
}

void ABlackoutPlayerState::ApplyBattleTransitionPolicy(EBattleTransitionType TransitionType)
{
	switch (TransitionType)
	{
	case EBattleTransitionType::LobbyToBattle:
		// 전투 시작: 소모품 초기값 유지, 능력 부여는 PlayerCharacter::PossessedBy에서 처리
		BO_LOG_CORE(Log, "ApplyBattleTransitionPolicy: LobbyToBattle for %s", *GetPlayerName());
		break;

	case EBattleTransitionType::CheckpointRest:
		// 체크포인트 휴식: HP 회복 등은 GE로 처리, 여기서는 상태 플래그만
		BO_LOG_CORE(Log, "ApplyBattleTransitionPolicy: CheckpointRest for %s", *GetPlayerName());
		break;

	case EBattleTransitionType::PartyWipeRestart:
		// 전멸 재시작: 소모품 초기값 복구
		SetConsumableCounts(1, 1);
		BO_LOG_CORE(Log, "ApplyBattleTransitionPolicy: PartyWipeRestart for %s", *GetPlayerName());
		break;
	}
}

void ABlackoutPlayerState::SetConsumableCounts(int32 NewBloodRootCount, int32 NewGulSerumCount)
{
	BloodRootCount = FMath::Max(0, NewBloodRootCount);
	GulSerumCount = FMath::Max(0, NewGulSerumCount);

	BroadcastConsumableCounts();
}

void ABlackoutPlayerState::InitializeConsumablesFromCharacterData(const UBOCharacterData* CharacterData)
{
	if (!CharacterData)
	{
		BO_LOG_CORE(Warning, "소모품 초기화 실패: CharacterData가 유효하지 않습니다. Player=%s", *GetPlayerName());
		return;
	}

	int32 InitialBloodRootCount = CharacterData->InitialBloodRoot;
	int32 InitialGulSerumCount = CharacterData->InitialGulSerum;

	for (const UBOConsumableData* ConsumableData : CharacterData->ConsumableSlots)
	{
		if (!ConsumableData || !ConsumableData->ConsumableTag.IsValid())
		{
			continue;
		}

		const int32 ClampedInitialCount = FMath::Clamp(ConsumableData->InitialCount, 0, ConsumableData->MaxCount);
		if (ConsumableData->ConsumableTag.MatchesTagExact(BlackoutGameplayTags::Item_Consumable_BloodRoot))
		{
			InitialBloodRootCount = ClampedInitialCount;
		}
		else if (ConsumableData->ConsumableTag.MatchesTagExact(BlackoutGameplayTags::Item_Consumable_GulSerum))
		{
			InitialGulSerumCount = ClampedInitialCount;
		}
	}

	SetConsumableCounts(InitialBloodRootCount, InitialGulSerumCount);
	BO_LOG_CORE(Log,
		"소모품 초기화: Player=%s BloodRoot=%d GulSerum=%d",
		*GetPlayerName(),
		BloodRootCount,
		GulSerumCount);
}

int32 ABlackoutPlayerState::GetConsumableCount(FGameplayTag ConsumableTag) const
{
	if (ConsumableTag.MatchesTagExact(BlackoutGameplayTags::Item_Consumable_BloodRoot))
	{
		return BloodRootCount;
	}

	if (ConsumableTag.MatchesTagExact(BlackoutGameplayTags::Item_Consumable_GulSerum))
	{
		return GulSerumCount;
	}

	return 0;
}

bool ABlackoutPlayerState::SetConsumableCount(FGameplayTag ConsumableTag, int32 NewCount)
{
	if (ConsumableTag.MatchesTagExact(BlackoutGameplayTags::Item_Consumable_BloodRoot))
	{
		BloodRootCount = FMath::Max(0, NewCount);
		BroadcastConsumableCounts();
		return true;
	}

	if (ConsumableTag.MatchesTagExact(BlackoutGameplayTags::Item_Consumable_GulSerum))
	{
		GulSerumCount = FMath::Max(0, NewCount);
		BroadcastConsumableCounts();
		return true;
	}

	BO_LOG_CORE(Warning, "소모품 수량 설정 실패: 지원하지 않는 태그입니다. Player=%s Tag=%s",
		*GetPlayerName(),
		*ConsumableTag.ToString());
	return false;
}

bool ABlackoutPlayerState::ConsumeConsumable(FGameplayTag ConsumableTag, int32 Amount)
{
	if (Amount <= 0)
	{
		BO_LOG_CORE(Warning, "소모품 소비 실패: 소비량이 0 이하입니다. Player=%s Tag=%s Amount=%d",
			*GetPlayerName(),
			*ConsumableTag.ToString(),
			Amount);
		return false;
	}

	const int32 CurrentCount = GetConsumableCount(ConsumableTag);
	if (CurrentCount < Amount)
	{
		BO_LOG_CORE(Warning, "소모품 소비 실패: 수량 부족. Player=%s Tag=%s Current=%d Amount=%d",
			*GetPlayerName(),
			*ConsumableTag.ToString(),
			CurrentCount,
			Amount);
		return false;
	}

	return SetConsumableCount(ConsumableTag, CurrentCount - Amount);
}

void ABlackoutPlayerState::OnRep_BloodRootCount()
{
	BroadcastConsumableCounts();
}

void ABlackoutPlayerState::OnRep_GulSerumCount()
{
	BroadcastConsumableCounts();
}

void ABlackoutPlayerState::BroadcastConsumableCounts()
{
	OnConsumableCountsChanged.Broadcast(BloodRootCount, GulSerumCount);
}
