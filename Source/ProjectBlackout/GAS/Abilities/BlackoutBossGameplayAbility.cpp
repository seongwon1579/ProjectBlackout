#include "GAS/Abilities/BlackoutBossGameplayAbility.h"

#include "BlackoutBossCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

const FName UBlackoutBossGameplayAbility::WarpTargetName = FName("MW_Target");

void UBlackoutBossGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	CachedOwner = Cast<ABlackoutBossCharacter>(ActorInfo->AvatarActor.Get());
	if (CachedOwner && PatternDataTag.IsValid())
	{
		CachedPatternData = CachedOwner->GetPatternData(PatternDataTag);
	}
}

void UBlackoutBossGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CachedPatternData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] No PatternData (key: %s)"),
		       *GetName(), *PatternDataTag.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	PreActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!TryResolveMontage(TriggerEventData))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	SetupEventListeners();
	PlayMontage();
	PostActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBlackoutBossGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              bool bReplicateEndAbility, bool bWasCancelled)
{
	if (CachedOwner)
	{
		if (UMotionWarpingComponent* WarpComp = CachedOwner->MotionWarpingComponent)
		{
			WarpComp->RemoveWarpTarget(WarpTargetName);
		}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


FGameplayTag UBlackoutBossGameplayAbility::SelectMontageTag(const FGameplayEventData* TriggerEventData) const
{
	if (CachedPatternData && CachedPatternData->Montages.Num() == 1)
	{
		return CachedPatternData->Montages.CreateConstIterator().Key();
	}
	return FGameplayTag::EmptyTag;
}

bool UBlackoutBossGameplayAbility::TryResolveMontage(const FGameplayEventData* TriggerEventData)
{
	const FGameplayTag MontageTag = SelectMontageTag(TriggerEventData);
	SelectedMontage = GetMontage(MontageTag);

	if (!SelectedMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to find montage"));
		return false;
	}
	return true;
}

void UBlackoutBossGameplayAbility::TrySetupMotionWarp(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	
	if (!CachedOwner || !CachedOwner->MotionWarpingComponent) return;

	const APawn* Target = Cast<const APawn>(TriggerEventData->Target.Get());
	if (!Target) return;

	CachedOwner->MotionWarpingComponent->AddOrUpdateWarpTargetFromComponent(
		WarpTargetName,
		Target->GetRootComponent(),
		NAME_None,
		true,
		FVector::ZeroVector
	);
}

void UBlackoutBossGameplayAbility::PlayMontage()
{
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		SelectedMontage,
		1.f,
		NAME_None,
		true,
		1.f
	);
	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutBossGameplayAbility::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutBossGameplayAbility::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutBossGameplayAbility::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

UAnimMontage* UBlackoutBossGameplayAbility::GetMontage(const FGameplayTag& Tag)
{
	if (!Tag.IsValid() || !CachedPatternData) return nullptr;

	const TObjectPtr<UAnimMontage>* Found = CachedPatternData->Montages.Find(Tag);
	return Found ? Found->Get() : nullptr;
}

void UBlackoutBossGameplayAbility::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
