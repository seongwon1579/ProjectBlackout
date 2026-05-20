#include "UI/BlackoutPartyRosterWidgetController.h"

#include "AbilitySystemComponent.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Core/BlackoutLog.h"
#include "EngineUtils.h"
#include "Framework/BlackoutGameState.h"
#include "Framework/BlackoutPlayerState.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTags/BlackoutGameplayTags.h"

void UBlackoutPartyRosterWidgetController::BeginDestroy()
{
	UnbindAllMembers();

	if (ABlackoutGameState* BlackoutGameState = GameState.Get())
	{
		BlackoutGameState->OnPlayerArrayChanged.RemoveAll(this);
	}

	Super::BeginDestroy();
}

bool UBlackoutPartyRosterWidgetController::Initialize(APlayerController* InPlayerController)
{
	if (!ResolveDependencies(InPlayerController))
	{
		return false;
	}

	BindCallbacksToDependencies();
	return true;
}

void UBlackoutPartyRosterWidgetController::BindCallbacksToDependencies()
{
	ABlackoutGameState* BlackoutGameState = GameState.Get();
	if (!BlackoutGameState)
	{
		BO_LOG_CORE(Warning, "파티 로스터 바인딩 보류: GameState가 유효하지 않습니다.");
		return;
	}

	if (!bGameStateCallbacksBound)
	{
		BlackoutGameState->OnPlayerArrayChanged.AddDynamic(
			this,
			&UBlackoutPartyRosterWidgetController::HandlePlayerArrayChanged);
		bGameStateCallbacksBound = true;
	}

	RefreshRoster();
}

void UBlackoutPartyRosterWidgetController::BroadcastInitialRoster()
{
	RefreshRoster();
}

void UBlackoutPartyRosterWidgetController::RefreshRoster()
{
	ABlackoutGameState* BlackoutGameState = GameState.Get();
	if (!BlackoutGameState)
	{
		OnRosterRebuilt.Broadcast(TArray<FBlackoutPartyMemberStatusData>());
		return;
	}

	TSet<ABlackoutPlayerState*> VisiblePlayerStates;
	TArray<FBlackoutPartyMemberStatusData> MemberStatusList;

	for (APlayerState* PlayerStateBase : BlackoutGameState->PlayerArray)
	{
		ABlackoutPlayerState* MemberPlayerState = Cast<ABlackoutPlayerState>(PlayerStateBase);
		if (!MemberPlayerState || MemberPlayerState == LocalPlayerState.Get())
		{
			continue;
		}

		VisiblePlayerStates.Add(MemberPlayerState);
		BindMember(MemberPlayerState);

		if (FBlackoutPartyMemberBinding* Binding = MemberBindings.Find(MemberPlayerState))
		{
			RefreshMemberCharacterBinding(*Binding);
		}

		MemberStatusList.Add(BuildStatusData(MemberPlayerState));
	}

	TArray<ABlackoutPlayerState*> RemovedPlayerStates;
	for (const auto& BindingPair : MemberBindings)
	{
		if (VisiblePlayerStates.Contains(BindingPair.Key))
		{
			continue;
		}

		RemovedPlayerStates.Add(BindingPair.Key);
	}

	for (ABlackoutPlayerState* RemovedPlayerState : RemovedPlayerStates)
	{
		UnbindMember(RemovedPlayerState);
	}

	OnRosterRebuilt.Broadcast(MemberStatusList);
}

bool UBlackoutPartyRosterWidgetController::ResolveDependencies(APlayerController* InPlayerController)
{
	if (!InPlayerController || !InPlayerController->IsLocalController())
	{
		BO_LOG_CORE(Error, "파티 로스터 초기화 실패: 로컬 PlayerController가 유효하지 않습니다.");
		return false;
	}

	ABlackoutPlayerState* BlackoutPlayerState = InPlayerController->GetPlayerState<ABlackoutPlayerState>();
	if (!BlackoutPlayerState)
	{
		BO_LOG_CORE(Verbose, "파티 로스터 초기화 보류: ABlackoutPlayerState 복제를 기다리는 중입니다.");
		return false;
	}

	ABlackoutGameState* BlackoutGameState =
		InPlayerController->GetWorld() ? InPlayerController->GetWorld()->GetGameState<ABlackoutGameState>() : nullptr;
	if (!BlackoutGameState)
	{
		BO_LOG_CORE(Verbose, "파티 로스터 초기화 보류: ABlackoutGameState 복제를 기다리는 중입니다.");
		return false;
	}

	if (GameState.Get() != BlackoutGameState)
	{
		if (ABlackoutGameState* PreviousGameState = GameState.Get())
		{
			PreviousGameState->OnPlayerArrayChanged.RemoveAll(this);
		}

		bGameStateCallbacksBound = false;
		UnbindAllMembers();
	}

	PlayerController = InPlayerController;
	LocalPlayerState = BlackoutPlayerState;
	GameState = BlackoutGameState;
	return true;
}

void UBlackoutPartyRosterWidgetController::BindMember(ABlackoutPlayerState* MemberPlayerState)
{
	if (!MemberPlayerState || MemberBindings.Contains(MemberPlayerState))
	{
		return;
	}

	FBlackoutPartyMemberBinding Binding;
	Binding.PlayerState = MemberPlayerState;
	Binding.AbilitySystemComponent = ResolveAbilitySystemComponent(MemberPlayerState);

	UAbilitySystemComponent* ASC = Binding.AbilitySystemComponent.Get();
	if (ASC)
	{
		const TWeakObjectPtr<ABlackoutPlayerState> WeakMemberPlayerState(MemberPlayerState);
		Binding.HealthChangedHandle =
			ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetHealthAttribute())
			.AddLambda([this, WeakMemberPlayerState](const FOnAttributeChangeData&)
			{
				if (ABlackoutPlayerState* BoundPlayerState = WeakMemberPlayerState.Get())
				{
					BroadcastMemberStatus(BoundPlayerState);
				}
			});

		Binding.MaxHealthChangedHandle =
			ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetMaxHealthAttribute())
			.AddLambda([this, WeakMemberPlayerState](const FOnAttributeChangeData&)
			{
				if (ABlackoutPlayerState* BoundPlayerState = WeakMemberPlayerState.Get())
				{
					BroadcastMemberStatus(BoundPlayerState);
				}
			});

		Binding.DownedStateTagChangedHandle =
			ASC->RegisterGameplayTagEvent(BlackoutGameplayTags::State_Downed, EGameplayTagEventType::NewOrRemoved)
			.AddLambda([this, WeakMemberPlayerState](const FGameplayTag, int32)
			{
				if (ABlackoutPlayerState* BoundPlayerState = WeakMemberPlayerState.Get())
				{
					BroadcastMemberStatus(BoundPlayerState);
				}
			});

		Binding.BeingRevivedStateTagChangedHandle =
			ASC->RegisterGameplayTagEvent(BlackoutGameplayTags::State_BeingRevived, EGameplayTagEventType::NewOrRemoved)
			.AddLambda([this, WeakMemberPlayerState](const FGameplayTag, int32)
			{
				if (ABlackoutPlayerState* BoundPlayerState = WeakMemberPlayerState.Get())
				{
					BroadcastMemberStatus(BoundPlayerState);
				}
			});

		Binding.DeadStateTagChangedHandle =
			ASC->RegisterGameplayTagEvent(BlackoutGameplayTags::State_Dead, EGameplayTagEventType::NewOrRemoved)
			.AddLambda([this, WeakMemberPlayerState](const FGameplayTag, int32)
			{
				if (ABlackoutPlayerState* BoundPlayerState = WeakMemberPlayerState.Get())
				{
					BroadcastMemberStatus(BoundPlayerState);
				}
			});
	}
	else
	{
		BO_LOG_CORE(Warning, "파티 멤버 ASC 바인딩 실패: PlayerState=%s", *GetNameSafe(MemberPlayerState));
	}

	MemberBindings.Add(MemberPlayerState, Binding);

	if (FBlackoutPartyMemberBinding* StoredBinding = MemberBindings.Find(MemberPlayerState))
	{
		RefreshMemberCharacterBinding(*StoredBinding);
	}
}

void UBlackoutPartyRosterWidgetController::UnbindMember(ABlackoutPlayerState* MemberPlayerState)
{
	FBlackoutPartyMemberBinding Binding;
	if (!MemberBindings.RemoveAndCopyValue(MemberPlayerState, Binding))
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = Binding.AbilitySystemComponent.Get())
	{
		if (Binding.HealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetHealthAttribute())
				.Remove(Binding.HealthChangedHandle);
		}

		if (Binding.MaxHealthChangedHandle.IsValid())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UBlackoutBaseAttributeSet::GetMaxHealthAttribute())
				.Remove(Binding.MaxHealthChangedHandle);
		}

		if (Binding.DownedStateTagChangedHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(BlackoutGameplayTags::State_Downed, EGameplayTagEventType::NewOrRemoved)
				.Remove(Binding.DownedStateTagChangedHandle);
		}

		if (Binding.BeingRevivedStateTagChangedHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(BlackoutGameplayTags::State_BeingRevived, EGameplayTagEventType::NewOrRemoved)
				.Remove(Binding.BeingRevivedStateTagChangedHandle);
		}

		if (Binding.DeadStateTagChangedHandle.IsValid())
		{
			ASC->RegisterGameplayTagEvent(BlackoutGameplayTags::State_Dead, EGameplayTagEventType::NewOrRemoved)
				.Remove(Binding.DeadStateTagChangedHandle);
		}
	}
}

void UBlackoutPartyRosterWidgetController::UnbindAllMembers()
{
	TArray<ABlackoutPlayerState*> BoundPlayerStates;
	MemberBindings.GetKeys(BoundPlayerStates);

	for (ABlackoutPlayerState* BoundPlayerState : BoundPlayerStates)
	{
		UnbindMember(BoundPlayerState);
	}
}

void UBlackoutPartyRosterWidgetController::RefreshMemberCharacterBinding(FBlackoutPartyMemberBinding& Binding)
{
	ABlackoutPlayerState* MemberPlayerState = Binding.PlayerState.Get();
	if (!MemberPlayerState)
	{
		return;
	}

	ABlackoutPlayerCharacter* NewPlayerCharacter = ResolvePlayerCharacter(MemberPlayerState);
	if (Binding.PlayerCharacter.Get() == NewPlayerCharacter)
	{
		return;
	}

	Binding.PlayerCharacter = NewPlayerCharacter;
}

void UBlackoutPartyRosterWidgetController::BroadcastMemberStatus(ABlackoutPlayerState* MemberPlayerState)
{
	if (!MemberPlayerState || !MemberBindings.Contains(MemberPlayerState))
	{
		return;
	}

	if (FBlackoutPartyMemberBinding* Binding = MemberBindings.Find(MemberPlayerState))
	{
		RefreshMemberCharacterBinding(*Binding);
	}

	OnMemberStatusChanged.Broadcast(BuildStatusData(MemberPlayerState));
}

FBlackoutPartyMemberStatusData UBlackoutPartyRosterWidgetController::BuildStatusData(
	ABlackoutPlayerState* MemberPlayerState) const
{
	FBlackoutPartyMemberStatusData StatusData;
	if (!MemberPlayerState)
	{
		return StatusData;
	}

	StatusData.PlayerState = MemberPlayerState;
	StatusData.DisplayName = FText::FromString(MemberPlayerState->GetPlayerName());
	StatusData.SelectedClassTag = MemberPlayerState->SelectedClassTag;
	StatusData.bIsLocalPlayer = MemberPlayerState == LocalPlayerState.Get();
	StatusData.bIsValid = true;

	const UAbilitySystemComponent* ASC = ResolveAbilitySystemComponent(MemberPlayerState);
	if (ASC)
	{
		StatusData.CurrentHealth = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
		StatusData.MaxHealth = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());
	}

	StatusData.bIsDowned = MemberPlayerState->IsDowned();
	StatusData.bIsReviveInteractionActive = MemberPlayerState->IsBeingRevived();

	const ABlackoutPlayerCharacter* PlayerCharacter = ResolvePlayerCharacter(MemberPlayerState);
	if (PlayerCharacter)
	{
		StatusData.bIsDead = PlayerCharacter->IsDead();
	}

	if (StatusData.bIsDowned)
	{
		StatusData.CurrentHealth = 0.0f;
	}

	StatusData.MaxHealth = FMath::Max(StatusData.MaxHealth, 0.0f);
	StatusData.CurrentHealth = StatusData.MaxHealth > 0.0f
		? FMath::Clamp(StatusData.CurrentHealth, 0.0f, StatusData.MaxHealth)
		: 0.0f;
	StatusData.NormalizedHealth = StatusData.MaxHealth > 0.0f
		? FMath::Clamp(StatusData.CurrentHealth / StatusData.MaxHealth, 0.0f, 1.0f)
		: 0.0f;

	return StatusData;
}

ABlackoutPlayerCharacter* UBlackoutPartyRosterWidgetController::ResolvePlayerCharacter(
	ABlackoutPlayerState* MemberPlayerState) const
{
	UWorld* World = PlayerController.IsValid() ? PlayerController->GetWorld() : nullptr;
	if (!World || !MemberPlayerState)
	{
		return nullptr;
	}

	for (TActorIterator<ABlackoutPlayerCharacter> It(World); It; ++It)
	{
		ABlackoutPlayerCharacter* Candidate = *It;
		if (Candidate && Candidate->GetPlayerState<ABlackoutPlayerState>() == MemberPlayerState)
		{
			return Candidate;
		}
	}

	return nullptr;
}

UAbilitySystemComponent* UBlackoutPartyRosterWidgetController::ResolveAbilitySystemComponent(
	ABlackoutPlayerState* MemberPlayerState) const
{
	return MemberPlayerState ? MemberPlayerState->GetAbilitySystemComponent() : nullptr;
}

void UBlackoutPartyRosterWidgetController::HandlePlayerArrayChanged()
{
	RefreshRoster();
}
