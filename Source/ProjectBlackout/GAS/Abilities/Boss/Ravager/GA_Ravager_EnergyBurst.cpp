#include "GAS/Abilities/Boss/Ravager//GA_Ravager_EnergyBurst.h"

#include "BlackoutBossCharacter.h"
#include "BlackoutDamageable.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "BlackoutGameplayTags.h"
#include "Engine/OverlapResult.h"

void UGA_Ravager_EnergyBurst::SetupEventListeners()
{
	if (WaitBeginEvent)
	{
		WaitBeginEvent->EndTask();
		WaitBeginEvent = nullptr;
	}
	if (WaitEndEvent)
	{
		WaitEndEvent->EndTask();
		WaitEndEvent = nullptr;
	}
    
	WaitBeginEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OnEnergyBurst,
		nullptr,
		true,
		false);
    
	if (WaitBeginEvent)
	{
		WaitBeginEvent->EventReceived.AddDynamic(this, &UGA_Ravager_EnergyBurst::OnEnergyBurstNotify);
		WaitBeginEvent->ReadyForActivation();
	}
	
	WaitEndEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
	this,
	BlackoutGameplayTags::Event_Enemy_Attack_OffEnergyBurst,
	nullptr,
	true,
	false);
    
	if (WaitEndEvent)
	{
		WaitEndEvent->EventReceived.AddDynamic(this, &UGA_Ravager_EnergyBurst::OffEnergyBurstNotify);
		WaitEndEvent->ReadyForActivation();
	}
}

void UGA_Ravager_EnergyBurst::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	World->GetTimerManager().ClearTimer(DamageTimer);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Ravager_EnergyBurst::OnEnergyBurstNotify(FGameplayEventData Payload)
{
	if (!IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] EnergyBurst invalid"), *GetName());
		return;
	}
	
	const FBossEnergyBurstSettings& Settings = CachedPatternData->EnergyBurstSettings;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	World->GetTimerManager().SetTimer(
	DamageTimer, this, &UGA_Ravager_EnergyBurst::ApplyDamage,
	Settings.DamageTickInterval, true);
}

void UGA_Ravager_EnergyBurst::OffEnergyBurstNotify(FGameplayEventData Payload)
{
	UWorld* World = GetWorld();
	if (!World) return;
	
	World->GetTimerManager().ClearTimer(DamageTimer);
}

void UGA_Ravager_EnergyBurst::ApplyDamage()
{
	if (!IsValid()) return;
	
	const FBossEnergyBurstSettings& Settings = CachedPatternData->EnergyBurstSettings;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedOwner);
	
	World->OverlapMultiByObjectType(
		Overlaps,
		CachedOwner->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Settings.DamageRadius),
		QueryParams);
	
	UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo();
	if (!OwnerASC) return;
	
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target) continue;
		
		IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Target);
		if (!Damageable) continue;
		
		FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
		EffectContext.AddSourceObject(CachedOwner);
		
		FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(
			Settings.DamageEffect, Settings.AbilityLevel, EffectContext);
		
		if (SpecHandle.IsValid())
		{
			const float Damage = Settings.Damage;
			
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, Damage);
			Damageable->ReceiveDamageFromHitbox(SpecHandle, NAME_None);
		}
	}
}

bool UGA_Ravager_EnergyBurst::IsValid() const
{
	return Super::IsValid() && CachedPatternData->EnergyBurstSettings.IsValid();
}
