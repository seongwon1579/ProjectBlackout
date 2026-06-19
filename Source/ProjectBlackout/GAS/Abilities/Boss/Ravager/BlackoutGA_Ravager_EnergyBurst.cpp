#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_EnergyBurst.h"

#include "BORavagerBoss.h"
#include "BlackoutDamageable.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "BlackoutGameplayTags.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

void UBlackoutGA_Ravager_EnergyBurst::PreActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                         const FGameplayAbilityActorInfo* ActorInfo,
                                                         const FGameplayAbilityActivationInfo ActivationInfo,
                                                         const FGameplayEventData* TriggerEventData)
{
	Super::PreActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UBlackoutGA_Ravager_EnergyBurst::SetupEventListeners()
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
		WaitBeginEvent->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_EnergyBurst::OnEnergyBurstNotify);
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
		WaitEndEvent->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_EnergyBurst::OffEnergyBurstNotify);
		WaitEndEvent->ReadyForActivation();
	}
}

void UBlackoutGA_Ravager_EnergyBurst::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                                 const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilityActivationInfo ActivationInfo,
                                                 bool bReplicateEndAbility, bool bWasCancelled)
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(DamageTimer);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UBlackoutGA_Ravager_EnergyBurst::OnEnergyBurstNotify(FGameplayEventData Payload)
{
	if (!CanActivatePattern())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] EnergyBurst invalid"), *GetName());
		return;
	}

	const FBossEnergyBurstSettings& Settings = CachedPatternData->EnergyBurstSettings;

	UWorld* World = GetWorld();
	if (!World) return;

	ApplyDamage();
	World->GetTimerManager().SetTimer(
		DamageTimer, this, &UBlackoutGA_Ravager_EnergyBurst::ApplyDamage,
		Settings.DamageTickInterval, true);
}

void UBlackoutGA_Ravager_EnergyBurst::OffEnergyBurstNotify(FGameplayEventData Payload)
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(DamageTimer);
}

void UBlackoutGA_Ravager_EnergyBurst::ApplyDamage()
{
	if (!CanActivatePattern()) return;

	const FBossEnergyBurstSettings& Settings = CachedPatternData->EnergyBurstSettings;

	UWorld* World = GetWorld();
	if (!World) return;

#if ENABLE_DRAW_DEBUG

	if (IsBossDebugEnabled())
	{
		const FVector Center = CachedOwner->GetActorLocation();
		DrawDebugSphere(
			World,
			Center,
			Settings.DamageRadius,
			24,
			FColor::Red,
			false,
			Settings.DamageTickInterval,
			0,
			2.0f);
	
		UE_LOG(LogTemp, Warning, TEXT("Current Damage: %f"), Settings.Damage)
	}

#endif

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

	TSet<AActor*> DamagedActors;  
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target) continue;
		
		if (DamagedActors.Contains(Target)) continue;
		DamagedActors.Add(Target);
		
		if (!ShouldDamageTarget(Target)) continue;

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
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Stun, Settings.StunMagnitude);
			Damageable->ReceiveDamageFromHitbox(SpecHandle, NAME_None);
		}
	}
}

bool UBlackoutGA_Ravager_EnergyBurst::HasValidSettings() const
{
	return CachedPatternData->EnergyBurstSettings.IsValid();
}
