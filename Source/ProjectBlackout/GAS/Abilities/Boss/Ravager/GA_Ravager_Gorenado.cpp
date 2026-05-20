#include "GAS/Abilities/Boss/Ravager//GA_Ravager_Gorenado.h"

#include "AbilitySystemComponent.h"
#include "BlackoutGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "BlackoutBossCharacter.h"
#include "BlackoutPullable.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Engine/OverlapResult.h"

void UGA_Ravager_Gorenado::SetupEventListeners()
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
		WaitBeginEvent->EventReceived.AddDynamic(this, &UGA_Ravager_Gorenado::OnPullStartNotify);
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
		WaitEndEvent->EventReceived.AddDynamic(this, &UGA_Ravager_Gorenado::OnPullEndNotify);
		WaitEndEvent->ReadyForActivation();
	}
}

void UGA_Ravager_Gorenado::UpdatePulling()
{
	if (!IsValid()) return;
	
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
	
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target) continue;
		
		if (!Cast<IBlackoutPullable>(Target)) continue;
		
		if (IsTargetBlocked(Target)) continue;;
		
		PullTarget(Target, UpdateInterval);
	}
}

void UGA_Ravager_Gorenado::ApplyDamage()
{
	if (!IsValid()) return;
	
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
	
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();
		if (!Target) continue;
		
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

bool UGA_Ravager_Gorenado::IsTargetBlocked(AActor* Target) const
{
	if (!Target|| !IsValid()) return false;
	
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

void UGA_Ravager_Gorenado::PullTarget(AActor* Target, float DeltaTime)
{
	if (!Target || !IsValid() ) return;
	
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

bool UGA_Ravager_Gorenado::IsValid() const
{
	return Super::IsValid() && CachedPatternData->GorenadoSettings.IsValid();
}

void UGA_Ravager_Gorenado::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
		World->GetTimerManager().ClearTimer(DamageTimer);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Ravager_Gorenado::OnPullStartNotify(FGameplayEventData Payload)
{
	if (!IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Gorenado invalid"), *GetName());
		return;
	}

	const FBossGorenadoSettings& Settings = CachedPatternData->GorenadoSettings;

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().SetTimer(
		UpdateTimer,
		this,
		&UGA_Ravager_Gorenado::UpdatePulling,
		UpdateInterval,
		true);

	World->GetTimerManager().SetTimer(
		DamageTimer, this, &UGA_Ravager_Gorenado::ApplyDamage,
		Settings.DamageTickInterval, true);
}

void UGA_Ravager_Gorenado::OnPullEndNotify(FGameplayEventData Payload)
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(UpdateTimer);
	World->GetTimerManager().ClearTimer(DamageTimer);
}
