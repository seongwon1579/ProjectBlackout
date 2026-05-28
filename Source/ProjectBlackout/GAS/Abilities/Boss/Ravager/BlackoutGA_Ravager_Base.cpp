#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"

#include "Characters/BlackoutBossCharacter.h"
#include "BlackoutGameplayTags.h"
#include "BORavagerBoss.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

const FName UBlackoutGA_Ravager_Base::WarpTargetName = FName("MW_Target");

UBlackoutGA_Ravager_Base::UBlackoutGA_Ravager_Base()
{
	ActivationOwnedTags.AddTag(BlackoutGameplayTags::Ability_PhaseLock);
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UBlackoutGA_Ravager_Base::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilitySpec& Spec)
{
	Super::OnAvatarSet(ActorInfo, Spec);

	CachedOwner = Cast<ABORavagerBoss>(ActorInfo->AvatarActor.Get());
	if (CachedOwner && PatternDataTag.IsValid())
	{
		CachedPatternData = CachedOwner->GetPatternData(PatternDataTag);
	}
}

void UBlackoutGA_Ravager_Base::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                   const FGameplayAbilityActorInfo* ActorInfo,
                                                   const FGameplayAbilityActivationInfo ActivationInfo,
                                                   const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CanActivatePattern())
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

void UBlackoutGA_Ravager_Base::EndAbility(const FGameplayAbilitySpecHandle Handle,
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


FGameplayTag UBlackoutGA_Ravager_Base::SelectMontageTag(const FGameplayEventData* TriggerEventData) const
{
	if (CachedPatternData && CachedPatternData->Montages.Num() == 1)
	{
		return CachedPatternData->Montages.CreateConstIterator().Key();
	}
	return FGameplayTag::EmptyTag;
}

bool UBlackoutGA_Ravager_Base::CanActivatePattern() const
{
	return CachedOwner && CachedPatternData && HasValidSettings();
}

bool UBlackoutGA_Ravager_Base::TryResolveMontage(const FGameplayEventData* TriggerEventData)
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

USceneComponent* UBlackoutGA_Ravager_Base::TrySetupMotionWarp(const FGameplayAbilitySpecHandle Handle,
                                                      const FGameplayAbilityActorInfo* ActorInfo,
                                                      const FGameplayAbilityActivationInfo ActivationInfo,
                                                      const FGameplayEventData* TriggerEventData)
{
	
	if (!CachedOwner || !CachedOwner->MotionWarpingComponent || !TriggerEventData) return nullptr;

	const APawn* Target = Cast<const APawn>(TriggerEventData->Target.Get());
	if (!Target) return nullptr;
	
	USceneComponent* TargetRoot = Target->GetRootComponent();
	if (!TargetRoot) return nullptr;

	CachedOwner->MotionWarpingComponent->AddOrUpdateWarpTargetFromComponent(
		WarpTargetName,
		TargetRoot,
		NAME_None,
		true,
		FVector::ZeroVector
	);
	
	return TargetRoot;
}

void UBlackoutGA_Ravager_Base::PlayMontage()
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
	MontageTask->OnCompleted.AddDynamic(this, &UBlackoutGA_Ravager_Base::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &UBlackoutGA_Ravager_Base::OnMontageEnded);
	MontageTask->OnCancelled.AddDynamic(this, &UBlackoutGA_Ravager_Base::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

UAnimMontage* UBlackoutGA_Ravager_Base::GetMontage(const FGameplayTag& Tag)
{
	if (!Tag.IsValid() || !CachedPatternData) return nullptr;

	const TObjectPtr<UAnimMontage>* Found = CachedPatternData->Montages.Find(Tag);
	return Found ? Found->Get() : nullptr;
}

void UBlackoutGA_Ravager_Base::OnMontageEnded()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
