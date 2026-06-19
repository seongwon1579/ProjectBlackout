#include "BlackoutPlayerState.h"
#include "BlackoutGameState.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Data/BOCharacterData.h"
#include "Data/BOConsumableData.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "BlackoutLog.h"
#include "AbilitySystemComponent.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GAS/Attributes/BlackoutPlayerAttributeSet.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"

ABlackoutPlayerState::ABlackoutPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<
		UBlackoutAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(
		EGameplayEffectReplicationMode::Mixed);

	BaseAttributeSet = CreateDefaultSubobject<UBlackoutBaseAttributeSet>(
		TEXT("BaseAttributeSet"));
	PlayerAttributeSet = CreateDefaultSubobject<UBlackoutPlayerAttributeSet>(
		TEXT("PlayerAttributeSet"));
	AmmoAttributeSet = CreateDefaultSubobject<UBlackoutAmmoAttributeSet>(
		TEXT("AmmoAttributeSet"));

	SetNetUpdateFrequency(100.f);
}

UAbilitySystemComponent* ABlackoutPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABlackoutPlayerState::CopyProperties(APlayerState* NewPlayerState)
{
	Super::CopyProperties(NewPlayerState);

	// Seamless Travel 로 새 PS 생성 시 선택 클래스 계승 (로비 → 보스맵).
	// 소모품/GAS 는 보스맵 PossessedBy 가 클래스 기반으로 재초기화하므로 복사 X.
	if (ABlackoutPlayerState* NewPS = Cast<ABlackoutPlayerState>(NewPlayerState))
	{
		NewPS->SelectedClassTag = SelectedClassTag;
		NewPS->bInfiniteHealthCheat = bInfiniteHealthCheat;
		NewPS->bInfiniteStaminaCheat = bInfiniteStaminaCheat;
		NewPS->bInfiniteAmmoCheat = bInfiniteAmmoCheat;
		NewPS->MatchStats = MatchStats;  
	}
}

void ABlackoutPlayerState::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlackoutPlayerState, SelectedClassTag);
	DOREPLIFETIME(ABlackoutPlayerState, BloodRootCount);
	DOREPLIFETIME(ABlackoutPlayerState, GulSerumCount);
	DOREPLIFETIME(ABlackoutPlayerState, bIsReady);
	DOREPLIFETIME(ABlackoutPlayerState, bRequestedSurrender);
	DOREPLIFETIME(ABlackoutPlayerState, bVotedAgainstSurrender);
	DOREPLIFETIME(ABlackoutPlayerState, MatchStats);
	DOREPLIFETIME_CONDITION(ABlackoutPlayerState, bInfiniteHealthCheat, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlackoutPlayerState, bInfiniteStaminaCheat, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ABlackoutPlayerState, bInfiniteAmmoCheat, COND_OwnerOnly);
}

void ABlackoutPlayerState::SetDebugCheatFlags(bool bNewInfiniteHealth, bool bNewInfiniteStamina, bool bNewInfiniteAmmo)
{
	const bool bFlagsChanged =
		bInfiniteHealthCheat != bNewInfiniteHealth
		|| bInfiniteStaminaCheat != bNewInfiniteStamina
		|| bInfiniteAmmoCheat != bNewInfiniteAmmo;

	bInfiniteHealthCheat = bNewInfiniteHealth;
	bInfiniteStaminaCheat = bNewInfiniteStamina;
	bInfiniteAmmoCheat = bNewInfiniteAmmo;

	ApplyActiveCheatState();

	if (bFlagsChanged)
	{
		BO_LOG_CORE(Log,
			"플레이어 치트 상태 갱신: Player=%s Health=%s Stamina=%s Ammo=%s",
			*GetPlayerName(),
			bInfiniteHealthCheat ? TEXT("On") : TEXT("Off"),
			bInfiniteStaminaCheat ? TEXT("On") : TEXT("Off"),
			bInfiniteAmmoCheat ? TEXT("On") : TEXT("Off"));
	}
}

void ABlackoutPlayerState::SetReadyState(bool bNewReady)
{
	if (!HasAuthority())
	{
		BO_LOG_NET(Warning, "SetReadyState 무시: 서버 권한이 없습니다. PlayerState=%s", *GetName());
		return;
	}

	if (bIsReady == bNewReady)
	{
		return;
	}

	bIsReady = bNewReady;
	BroadcastReadyStateChanged();
}

void ABlackoutPlayerState::SetLoadedState(bool bNewLoaded)
{
	if (!HasAuthority())
	{
		BO_LOG_NET(Warning, "SetLoadedState 무시: 서버 권한 없음. PlayerState=%s", *GetName());
		return;
	}
	
	bIsLoaded = bNewLoaded;
}

void ABlackoutPlayerState::ApplyBattleTransitionPolicy(
	EBattleTransitionType TransitionType)
{
	switch (TransitionType)
	{
	case EBattleTransitionType::LobbyToBattle:
		// 전투 시작: 소모품 초기값 유지, 능력 부여는 PlayerCharacter::PossessedBy에서 처리
		BO_LOG_CORE(Log, "ApplyBattleTransitionPolicy: LobbyToBattle for %s",
		            *GetPlayerName());
		break;

	case EBattleTransitionType::CheckpointRest:
		// 체크포인트 휴식: HP 회복 등은 GE로 처리, 여기서는 상태 플래그만
		RestoreAtCheckpoint();
		BO_LOG_CORE(Log, "ApplyBattleTransitionPolicy: CheckpointRest for %s",
		            *GetPlayerName());
		break;

	case EBattleTransitionType::PartyWipeRestart:
		// 전멸 재시작: 소모품 초기값 복구
		SetConsumableCounts(1, 1);
		BO_LOG_CORE(Log, "ApplyBattleTransitionPolicy: PartyWipeRestart for %s",
		            *GetPlayerName());
		break;

	case EBattleTransitionType::SurrenderRestart:
		// 항복 재시작: 소모품 초기값 복구
		SetConsumableCounts(1, 1);
		BO_LOG_CORE(Log, "ApplyBattleTransitionPolicy: SurrenderRestart for %s",
		            *GetPlayerName());
		break;
	}
}

void ABlackoutPlayerState::SetConsumableCounts(int32 NewBloodRootCount,
                                               int32 NewGulSerumCount)
{
	BloodRootCount = FMath::Max(0, NewBloodRootCount);
	GulSerumCount = FMath::Max(0, NewGulSerumCount);

	BroadcastConsumableCounts();
}

void ABlackoutPlayerState::InitializeConsumablesFromCharacterData(
	const UBOCharacterData* CharacterData)
{
	if (!CharacterData)
	{
		BO_LOG_CORE(Warning, "소모품 초기화 실패: CharacterData가 유효하지 않습니다. Player=%s",
		            *GetPlayerName());
		return;
	}

	int32 InitialBloodRootCount = CharacterData->InitialBloodRoot;
	int32 InitialGulSerumCount = CharacterData->InitialGulSerum;

	for (const UBOConsumableData* ConsumableData : CharacterData->
	     ConsumableSlots)
	{
		if (!ConsumableData || !ConsumableData->ConsumableTag.IsValid())
		{
			continue;
		}

		const int32 ClampedInitialCount = FMath::Clamp(
			ConsumableData->InitialCount, 0, ConsumableData->MaxCount);
		if (ConsumableData->ConsumableTag.MatchesTagExact(
			BlackoutGameplayTags::Item_Consumable_BloodRoot))
		{
			InitialBloodRootCount = ClampedInitialCount;
		}
		else if (ConsumableData->ConsumableTag.MatchesTagExact(
			BlackoutGameplayTags::Item_Consumable_GulSerum))
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
	if (ConsumableTag.MatchesTagExact(
		BlackoutGameplayTags::Item_Consumable_BloodRoot))
	{
		return BloodRootCount;
	}

	if (ConsumableTag.MatchesTagExact(
		BlackoutGameplayTags::Item_Consumable_GulSerum))
	{
		return GulSerumCount;
	}

	return 0;
}

bool ABlackoutPlayerState::SetConsumableCount(FGameplayTag ConsumableTag,
                                              int32 NewCount)
{
	if (ConsumableTag.MatchesTagExact(
		BlackoutGameplayTags::Item_Consumable_BloodRoot))
	{
		BloodRootCount = FMath::Max(0, NewCount);
		BroadcastConsumableCounts();
		return true;
	}

	if (ConsumableTag.MatchesTagExact(
		BlackoutGameplayTags::Item_Consumable_GulSerum))
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

bool ABlackoutPlayerState::ConsumeConsumable(FGameplayTag ConsumableTag,
                                             int32 Amount)
{
	if (Amount <= 0)
	{
		BO_LOG_CORE(Warning,
		            "소모품 소비 실패: 소비량이 0 이하입니다. Player=%s Tag=%s Amount=%d",
		            *GetPlayerName(),
		            *ConsumableTag.ToString(),
		            Amount);
		return false;
	}

	const int32 CurrentCount = GetConsumableCount(ConsumableTag);
	if (CurrentCount < Amount)
	{
		BO_LOG_CORE(Warning,
		            "소모품 소비 실패: 수량 부족. Player=%s Tag=%s Current=%d Amount=%d",
		            *GetPlayerName(),
		            *ConsumableTag.ToString(),
		            CurrentCount,
		            Amount);
		return false;
	}

	return SetConsumableCount(ConsumableTag, CurrentCount - Amount);
}

bool ABlackoutPlayerState::IsDowned() const
{
	return HasStateTag(BlackoutGameplayTags::State_Downed);
}

bool ABlackoutPlayerState::IsReviving() const
{
	return HasStateTag(BlackoutGameplayTags::State_Reviving);
}

bool ABlackoutPlayerState::IsBeingRevived() const
{
	return HasStateTag(BlackoutGameplayTags::State_BeingRevived);
}

void ABlackoutPlayerState::AddDamageDealt(float Amount)
{
	if (!HasAuthority() || Amount <= 0.f)
	{
		return;
	}
	MatchStats.DamageDealt += FMath::RoundToInt(Amount);   
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::RecordKill(bool bWasMeleeKill)
{
	if (!HasAuthority())
	{
		return;
	}
	++MatchStats.Kills;
	if (bWasMeleeKill)
	{
		++MatchStats.MeleeKills;
	}
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::RecordShotsFired(int32 Count)
{
	if (!HasAuthority() || Count <= 0)
	{
		return;
	}
	MatchStats.ShotsFired += Count;
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::RecordShotsHit(int32 Count)
{
	if (!HasAuthority() || Count <= 0)
	{
		return;
	}
	MatchStats.ShotsHit += Count;  
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::RecordConsumableUsed()
{
	if (!HasAuthority())
	{
		return;
	}
	++MatchStats.ConsumablesUsed;
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::RecordRevive()
{
	if (!HasAuthority())
	{
		return;
	}
	++MatchStats.Revives;
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::OnRep_MatchStats()
{
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	OnPlayerNameChangedNative.Broadcast();   // 닉네임 늦게 도착 → 로스터 그 멤버 갱신
}

void ABlackoutPlayerState::OnRep_IsReady()
{
	BroadcastReadyStateChanged();
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

void ABlackoutPlayerState::BroadcastMatchStatsChanged()
{
	OnMatchStatsChangedNative.Broadcast();
}

void ABlackoutPlayerState::SnapshotMatchStats()
{
	if (!HasAuthority())
	{
		return;
	}
	CheckpointStats = MatchStats;
}

void ABlackoutPlayerState::RollbackMatchStats()
{
	if (!HasAuthority())
	{
		return;
	}
	MatchStats = CheckpointStats;
	BroadcastMatchStatsChanged();
}

void ABlackoutPlayerState::BroadcastReadyStateChanged()
{
	OnReadyStateChangedNative.Broadcast(bIsReady);
}

bool ABlackoutPlayerState::HasStateTag(FGameplayTag StateTag) const
{
	return AbilitySystemComponent
		&& StateTag.IsValid()
		&& AbilitySystemComponent->HasMatchingGameplayTag(StateTag);
}

void ABlackoutPlayerState::RestoreAtCheckpoint()
{
	if (!HasAuthority())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// 체력 회복
	const float MaxHealth = ASC->GetNumericAttribute(
		UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	if (MaxHealth > 0.0f)
	{
		ASC->SetNumericAttributeBase(
			UBlackoutBaseAttributeSet::GetHealthAttribute(), MaxHealth);
	}

	// 스태미나 회복
	const float MaxStamina = ASC->GetNumericAttribute(
		UBlackoutPlayerAttributeSet::GetMaxStaminaAttribute());
	if (MaxStamina > 0.0f)
	{
		ASC->SetNumericAttributeBase(
			UBlackoutPlayerAttributeSet::GetStaminaAttribute(), MaxStamina);
	}

	// 유물 회복
	const float MaxRelicCharges = ASC->GetNumericAttribute(
		UBlackoutPlayerAttributeSet::GetMaxRelicChargesAttribute());
	if (MaxRelicCharges > 0.0f)
	{
		ASC->SetNumericAttributeBase(
			UBlackoutPlayerAttributeSet::GetRelicChargesAttribute(),
			MaxRelicCharges);
	}

	// 탄약 회복
	const float PrimaryMaxClip = ASC->GetNumericAttribute(
		UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute());
	if (PrimaryMaxClip > 0.0f)
	{
		ASC->SetNumericAttributeBase(
			UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute(),
			PrimaryMaxClip);
	}

	const float SecondaryMaxClip = ASC->GetNumericAttribute(
		UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute());
	if (SecondaryMaxClip > 0.0f)
	{
		ASC->SetNumericAttributeBase(
			UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute(),
			SecondaryMaxClip);
	}

	// 예비탄약 + 소모품 회복 — PC/CombatComp 체인을 한 번만 거친다.
	const APawn* OwnedPawn = GetPawn();
	const ABlackoutPlayerCharacter* PC = OwnedPawn
		? Cast<ABlackoutPlayerCharacter>(OwnedPawn)
		: nullptr;
	if (!PC)
	{
		return;
	}

	if (const UBlackoutCombatComponent* CombatComp = PC->GetCombatComponent())
	{
		if (const ABOFirearm* Primary = CombatComp->GetPrimaryFirearm())
		{
			ASC->SetNumericAttributeBase(
				UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute(),
				static_cast<float>(Primary->GetMaxReserveAmmo()));
		}
		if (const ABOFirearm* Secondary = CombatComp->GetSecondaryFirearm())
		{
			ASC->SetNumericAttributeBase(
				UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute(),
				static_cast<float>(Secondary->GetMaxReserveAmmo()));
		}
	}

	if (const UBOCharacterData* CharData = PC->GetCharacterData())
	{
		InitializeConsumablesFromCharacterData(CharData);
	}
}

void ABlackoutPlayerState::OnRep_SurrenderVoteState()
{
	if (ABlackoutGameState* GS = GetWorld() ? GetWorld()->GetGameState<ABlackoutGameState>() : nullptr)
	{
		GS->OnSurrenderVoteStateChanged.Broadcast(
			GS->bIsSurrenderVoteActive,
			GS->SurrenderVoteYesCount,
			GS->SurrenderVoteNoCount,
			GS->SurrenderVoteEndTimeSeconds
		);
	}
}

void ABlackoutPlayerState::OnRep_DebugCheatFlags()
{
	ApplyActiveCheatState();
}

void ABlackoutPlayerState::ApplyActiveCheatState()
{
	if (bInfiniteHealthCheat)
	{
		ApplyInfiniteHealthCheat();
	}

	if (bInfiniteStaminaCheat)
	{
		ApplyInfiniteStaminaCheat();
	}

	if (bInfiniteAmmoCheat)
	{
		ApplyInfiniteAmmoCheat();
	}
}

void ABlackoutPlayerState::ApplyInfiniteHealthCheat() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const float MaxHealth = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	if (MaxHealth > 0.0f)
	{
		ASC->SetNumericAttributeBase(UBlackoutBaseAttributeSet::GetHealthAttribute(), MaxHealth);
	}
}

void ABlackoutPlayerState::ApplyInfiniteStaminaCheat() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const float MaxStamina = ASC->GetNumericAttribute(UBlackoutPlayerAttributeSet::GetMaxStaminaAttribute());
	if (MaxStamina > 0.0f)
	{
		ASC->SetNumericAttributeBase(UBlackoutPlayerAttributeSet::GetStaminaAttribute(), MaxStamina);
	}
}

void ABlackoutPlayerState::ApplyInfiniteAmmoCheat() const
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const float PrimaryMaxClip = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryMaxClipAttribute());
	if (PrimaryMaxClip > 0.0f)
	{
		ASC->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute(), PrimaryMaxClip);
	}

	const float SecondaryMaxClip = ASC->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryMaxClipAttribute());
	if (SecondaryMaxClip > 0.0f)
	{
		ASC->SetNumericAttributeBase(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute(), SecondaryMaxClip);
	}

	const APawn* OwnedPawn = GetPawn();
	const ABlackoutPlayerCharacter* PlayerCharacter = OwnedPawn ? Cast<ABlackoutPlayerCharacter>(OwnedPawn) : nullptr;
	const UBlackoutCombatComponent* CombatComp = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	if (!CombatComp)
	{
		return;
	}

	if (const ABOFirearm* Primary = CombatComp->GetPrimaryFirearm())
	{
		ASC->SetNumericAttributeBase(
			UBlackoutAmmoAttributeSet::GetPrimaryReserveAmmoAttribute(),
			static_cast<float>(Primary->GetMaxReserveAmmo()));
	}

	if (const ABOFirearm* Secondary = CombatComp->GetSecondaryFirearm())
	{
		ASC->SetNumericAttributeBase(
			UBlackoutAmmoAttributeSet::GetSecondaryReserveAmmoAttribute(),
			static_cast<float>(Secondary->GetMaxReserveAmmo()));
	}
}
