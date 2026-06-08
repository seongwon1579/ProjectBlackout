#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Gorenado.h"

#include "AbilitySystemComponent.h"
#include "BlackoutGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "BORavagerBoss.h"
#include "BlackoutPullable.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Engine/OverlapResult.h"

void UBlackoutGA_Ravager_Gorenado::SetupEventListeners()
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
		BlackoutGameplayTags::Event_Enemy_Attack_OnGorenado,
		nullptr,
		true,
		false);

	if (WaitBeginEvent)
	{
		WaitBeginEvent->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_Gorenado::OnPullStartNotify);
		WaitBeginEvent->ReadyForActivation();
	}

	WaitEndEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_OffGorenado,
		nullptr,
		true,
		false);

	if (WaitEndEvent)
	{
		WaitEndEvent->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_Gorenado::OnPullEndNotify);
		WaitEndEvent->ReadyForActivation();
	}
}

void UBlackoutGA_Ravager_Gorenado::UpdatePulling()
{
	if (!CanActivatePattern()) return;
	
	const FBossGorenadoSettings& Settings = CachedPatternData->GorenadoSettings;
	
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
		FCollisionShape::MakeSphere(Settings.PullRadius),
		QueryParams);
	
	TSet<AActor*> PulledThisUpdate; 
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target) continue;
		
		if (!Cast<IBlackoutPullable>(Target)) continue;
		
		if (PulledThisUpdate.Contains(Target)) continue; 
		PulledThisUpdate.Add(Target);
		
		SetBeingPulledTag(Target, true);
		PulledActors.Add(Target);
		
		if (IsTargetBlocked(Target)) continue;;
		
		PullTarget(Target, UpdateInterval);
	}
	
	for (auto It = PulledActors.CreateIterator(); It; ++It)
	{
		AActor* Prev = It->Get();
		if (!Prev || !PulledThisUpdate.Contains(Prev))
		{
			if (Prev)
			{
				SetBeingPulledTag(Prev, false);
			}
			It.RemoveCurrent();
		}
	}
}

void UBlackoutGA_Ravager_Gorenado::ApplyDamage()
{
	if (!CanActivatePattern()) return;
	
	const FBossGorenadoSettings& Settings = CachedPatternData->GorenadoSettings;
	
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
	
	TSet<AActor*> DamagedActors; 
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target) continue;
		
		if (!ShouldDamageTarget(Target)) continue;
		
		if (DamagedActors.Contains(Target)) continue;
		DamagedActors.Add(Target);
		
		IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(Target);
		if (!Damageable) continue;
		
		if (IsTargetBlocked(Target)) continue;
		
		FGameplayEffectContextHandle EffectContext = OwnerASC->MakeEffectContext();
		EffectContext.AddSourceObject(CachedOwner);
		
		FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(
			Settings.DamageEffect, Settings.AbilityLevel, EffectContext);
		
		if (SpecHandle.IsValid())
		{
			const float Damage = Settings.Damage * Settings.DamageTickInterval;
			
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, Damage);
			Damageable->ReceiveDamageFromHitbox(SpecHandle, NAME_None);
		}
	}
}

bool UBlackoutGA_Ravager_Gorenado::IsTargetBlocked(AActor* Target) const
{
	if (!Target|| !CanActivatePattern()) return false;
	
	UWorld* World = GetWorld();
	if (!World) return false;
	
	const FBossGorenadoSettings& Settings = CachedPatternData->GorenadoSettings;
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(CachedOwner);
	Params.AddIgnoredActor(Target);
	
	return World->LineTraceSingleByChannel(
		Hit,
		CachedOwner->GetActorLocation(),
		Target->GetActorLocation(),
		ECC_WorldStatic,
		Params);
}

void UBlackoutGA_Ravager_Gorenado::PullTarget(AActor* Target, float DeltaTime)
{
	if (!Target || !CanActivatePattern() ) return;
	
	const FBossGorenadoSettings& Settings = CachedPatternData->GorenadoSettings;
	
	FVector PullDirection = CachedOwner->GetActorLocation() - Target->GetActorLocation();
	const float Distance = PullDirection.Size();
	
	if (Distance < Settings.MinDistance) return;
	
	PullDirection.Normalize();
	
	FPullData PullData;
	PullData.PullDirection = PullDirection;
	PullData.PullStrength = Settings.PullStrength;
	PullData.DeltaTime = DeltaTime;
	PullData.Instigator = CachedOwner;
	
	if (IBlackoutPullable* Pullable = Cast<IBlackoutPullable>(Target))
	{
		Pullable->ApplyPull(PullData);
	}
}

void UBlackoutGA_Ravager_Gorenado::SetBeingPulledTag(AActor* Target, bool bApply)
{
	if (!Target) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	if (!ASI) return;

	UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
	if (!TargetASC) return;

	if (bApply)
	{
		if (!TargetASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Pulled))
		{
			TargetASC->AddLooseGameplayTag(BlackoutGameplayTags::State_Pulled);
		}
	}
	else
	{
		TargetASC->RemoveLooseGameplayTag(BlackoutGameplayTags::State_Pulled);
	}
}

void UBlackoutGA_Ravager_Gorenado::ClearAllPulledTags()
{
	for (const TWeakObjectPtr<AActor>& WeakActor : PulledActors)
	{
		if (AActor* Target = WeakActor.Get())
		{
			SetBeingPulledTag(Target, false);
		}
	}
	PulledActors.Empty();
}

void UBlackoutGA_Ravager_Gorenado::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                              bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
		World->GetTimerManager().ClearTimer(DamageTimer);
	}

	if (UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo())
	{
		OwnerASC->RemoveGameplayCue(BlackoutGameplayTags::GameplayCue_Ravager_Gorenado);
	}
	
	ClearAllPulledTags();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UBlackoutGA_Ravager_Gorenado::HasValidSettings() const
{
	return CachedPatternData->GorenadoSettings.IsValid();
}

void UBlackoutGA_Ravager_Gorenado::OnPullStartNotify(FGameplayEventData Payload)
{
	if (!CanActivatePattern()) return;

	const FBossGorenadoSettings& Settings = CachedPatternData->GorenadoSettings;

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().SetTimer(
		UpdateTimer,
		this,
		&UBlackoutGA_Ravager_Gorenado::UpdatePulling,
		UpdateInterval,
		true);

	ApplyDamage();
	World->GetTimerManager().SetTimer(
		DamageTimer, this, &UBlackoutGA_Ravager_Gorenado::ApplyDamage,
		Settings.DamageTickInterval, true);

	if (UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo())
	{
		FGameplayCueParameters CueParams;
		CueParams.Instigator = CachedOwner;
		CueParams.EffectCauser = CachedOwner;
		OwnerASC->AddGameplayCue(BlackoutGameplayTags::GameplayCue_Ravager_Gorenado, CueParams);
	}
}

void UBlackoutGA_Ravager_Gorenado::OnPullEndNotify(FGameplayEventData Payload)
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(UpdateTimer);
	World->GetTimerManager().ClearTimer(DamageTimer);

	if (UAbilitySystemComponent* OwnerASC = GetAbilitySystemComponentFromActorInfo())
	{
		OwnerASC->RemoveGameplayCue(BlackoutGameplayTags::GameplayCue_Ravager_Gorenado);
	}
	
	ClearAllPulledTags();
	PulledActors.Empty();
}
