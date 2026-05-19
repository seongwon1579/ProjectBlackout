#include "UI/BlackoutPartyRosterWidgetController.h"

#include "AbilitySystemComponent.h"
#include "Characters/BlackoutCharacterBase.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Core/BlackoutLog.h"
#include "EngineUtils.h"
#include "Framework/BlackoutGameState.h"
#include "Framework/BlackoutPlayerState.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "GameFramework/PlayerController.h"

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
	}

	if (ABlackoutPlayerCharacter* PlayerCharacter = Binding.PlayerCharacter.Get())
	{
		if (Binding.DownedStateChangedHandle.IsValid())
		{
			PlayerCharacter->OnDownedStateChangedNative.Remove(Binding.DownedStateChangedHandle);
		}

		if (Binding.ReviveInteractionStateChangedHandle.IsValid())
		{
			PlayerCharacter->OnReviveInteractionStateChangedNative.Remove(Binding.ReviveInteractionStateChangedHandle);
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

	if (ABlackoutPlayerCharacter* PreviousPlayerCharacter = Binding.PlayerCharacter.Get())
	{
		if (Binding.DownedStateChangedHandle.IsValid())
		{
			PreviousPlayerCharacter->OnDownedStateChangedNative.Remove(Binding.DownedStateChangedHandle);
			Binding.DownedStateChangedHandle.Reset();
		}

		if (Binding.ReviveInteractionStateChangedHandle.IsValid())
		{
			PreviousPlayerCharacter->OnReviveInteractionStateChangedNative.Remove(
				Binding.ReviveInteractionStateChangedHandle);
			Binding.ReviveInteractionStateChangedHandle.Reset();
		}
	}

	Binding.PlayerCharacter = NewPlayerCharacter;

	if (NewPlayerCharacter)
	{
		Binding.DownedStateChangedHandle = NewPlayerCharacter->OnDownedStateChangedNative.AddUObject(
			this,
			&UBlackoutPartyRosterWidgetController::HandleMemberDownedStateChanged);
		Binding.ReviveInteractionStateChangedHandle =
			NewPlayerCharacter->OnReviveInteractionStateChangedNative.AddUObject(
				this,
				&UBlackoutPartyRosterWidgetController::HandleMemberReviveInteractionStateChanged);
	}
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

	const ABlackoutPlayerCharacter* PlayerCharacter = ResolvePlayerCharacter(MemberPlayerState);
	if (PlayerCharacter)
	{
		StatusData.bIsDowned = PlayerCharacter->IsDowned();
		StatusData.bIsReviveInteractionActive = PlayerCharacter->IsReviveInteractionActive();
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

void UBlackoutPartyRosterWidgetController::HandleMemberDownedStateChanged(
	ABlackoutCharacterBase* ChangedCharacter,
	bool bIsDowned)
{
	ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ChangedCharacter);
	ABlackoutPlayerState* MemberPlayerState =
		PlayerCharacter ? PlayerCharacter->GetPlayerState<ABlackoutPlayerState>() : nullptr;
	if (!MemberPlayerState)
	{
		return;
	}

	BroadcastMemberStatus(MemberPlayerState);
}

void UBlackoutPartyRosterWidgetController::HandleMemberReviveInteractionStateChanged(
	ABlackoutPlayerCharacter* ChangedCharacter,
	bool bIsReviveInteractionActive)
{
	ABlackoutPlayerState* MemberPlayerState =
		ChangedCharacter ? ChangedCharacter->GetPlayerState<ABlackoutPlayerState>() : nullptr;
	if (!MemberPlayerState)
	{
		return;
	}

	BroadcastMemberStatus(MemberPlayerState);
}
