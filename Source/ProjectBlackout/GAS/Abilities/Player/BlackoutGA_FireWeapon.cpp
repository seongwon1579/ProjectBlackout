#include "GAS/Abilities/Player/BlackoutGA_FireWeapon.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Components/BlackoutImpactIndicatorComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOShotgunFirearm.h"
#include "Combat/Weapons/BOWeaponDebugUtils.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Core/BlackoutLog.h"
#include "DrawDebugHelpers.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

namespace
{
	constexpr float PredictedFireDebugTraceDistance = 10000.0f;

	void DrawPredictedFireDebugLine(
		UWorld* World,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		const AActor* IgnoredOwner,
		const AActor* IgnoredWeapon,
		const float Duration,
		const float Thickness)
	{
		if (!World)
		{
			return;
		}

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutGA_FireWeapon_PredictedDebug), false, IgnoredOwner);
		if (IgnoredWeapon)
		{
			QueryParams.AddIgnoredActor(IgnoredWeapon);
		}

		FHitResult PredictedHitResult;
		World->LineTraceSingleByChannel(
			PredictedHitResult,
			TraceStart,
			TraceEnd,
			BlackoutCollisionChannels::WeaponTrace,
			QueryParams);

		AActor* DamageTargetActor = BlackoutWeaponDebug::ResolveDamageTargetActor(PredictedHitResult);
		const FVector DebugEnd = PredictedHitResult.bBlockingHit ? PredictedHitResult.ImpactPoint : TraceEnd;
		const FColor DebugColor = BlackoutWeaponDebug::GetHitscanDebugColor(PredictedHitResult.bBlockingHit, DamageTargetActor);
		DrawDebugLine(World, TraceStart, DebugEnd, DebugColor, false, Duration, 0, Thickness);
	}
}

UBlackoutGA_FireWeapon::UBlackoutGA_FireWeapon()
{
	InputID = EBlackoutAbilityInputID::Fire;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	ActivationRequiredTags.AddTag(BlackoutGameplayTags::State_Aiming);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Reloading);
	
	// TODO: LobbyTag.InfiniteAmmo 분기로 탄약 소모 체크 생략 로직 추가 (TDD §7.1)
}

void UBlackoutGA_FireWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Log, "GA_FireWeapon activate requested");

	ABlackoutPlayerCharacter* PlayerCharacter = ActorInfo ? Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	const UBlackoutImpactIndicatorComponent* ImpactIndicatorComponent = PlayerCharacter ? PlayerCharacter->GetImpactIndicatorComponent() : nullptr;
	ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
	if (!EquippedFirearm || !ImpactIndicatorComponent)
	{
		BO_LOG_GAS(Warning,
		           "GA_FireWeapon failed: Firearm=%s ImpactIndicatorComponent=%s",
		           *GetNameSafe(EquippedFirearm),
		           *GetNameSafe(ImpactIndicatorComponent));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 단발/반자동 무기는 몽타주 유무와 무관하게 발사 간격을 강제로 보장합니다.
	if (!CanFireAtCurrentTime(EquippedFirearm, ActorInfo))
	{
		const float CurrentTimeSeconds = ActorInfo && ActorInfo->AvatarActor.IsValid() && ActorInfo->AvatarActor->GetWorld()
			? ActorInfo->AvatarActor->GetWorld()->GetTimeSeconds()
			: -1.0f;
		BO_LOG_GAS(Log,
			"GA_FireWeapon skipped by fire-rate gate: Character=%s Weapon=%s Current=%.3f Next=%.3f",
			*GetNameSafe(PlayerCharacter),
			*GetNameSafe(EquippedFirearm),
			CurrentTimeSeconds,
			NextAllowedFireTimeSeconds);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		BO_LOG_GAS(Warning, "GA_FireWeapon failed: CommitAbility 실패");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 1. 탄약 소모 확인 및 적용
	if (!ApplyAmmoCost())
	{
		BO_LOG_GAS(Warning, "GA_FireWeapon failed: 탄약 부족");
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ReserveNextFireTime(EquippedFirearm, ActorInfo);

	BO_LOG_GAS(Log, "GA_FireWeapon activated: Character=%s, Weapon=%s", *GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr), *GetNameSafe(EquippedFirearm));

	const FGameplayTag FireAnimTag = EquippedFirearm->GetFireAnimTag();
	CachedFireMontage = PlayerCharacter ? PlayerCharacter->GetFireMontageForTag(FireAnimTag) : nullptr;
	bWeaponFireAnimationTriggered = false;

	if (FireAnimTag.IsValid() && !CachedFireMontage)
	{
		BO_LOG_GAS(Warning,
			"GA_FireWeapon fire montage lookup failed: Character=%s Weapon=%s FireAnimTag=%s",
			*GetNameSafe(PlayerCharacter),
			*GetNameSafe(EquippedFirearm),
			*FireAnimTag.ToString());
	}

	if (CachedFireMontage)
	{
		if (UAbilityTask_WaitGameplayEvent* WaitEventTask =
			UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, BlackoutGameplayTags::Event_Montage_FireWeaponStart))
		{
			WaitEventTask->EventReceived.AddDynamic(this, &UBlackoutGA_FireWeapon::OnWeaponFireStartEventReceived);
			WaitEventTask->ReadyForActivation();
		}
	}

	// 2. 사격 애니메이션 몽타주 재생
	PlayFireMontage();

	const FVector MuzzleLocation = CombatComponent->GetMuzzleTransform().GetLocation();
	const FVector AimTarget = ImpactIndicatorComponent->GetAimTargetPoint();
	const FVector BaseFireDirection = (AimTarget - MuzzleLocation).GetSafeNormal();
	const FVector FireDirection = CombatComponent->GetSpreadDeviatedDirection(BaseFireDirection);

	// 로컬 예측 디버그는 실제 판정을 복제하지 않고 현재 클라가 기대하는 발사선을 즉시 시각화합니다.
	if (PlayerCharacter->IsLocallyControlled() && EquippedFirearm->UsesHitscan() && EquippedFirearm->ShouldDrawDebugHitscanRay())
	{
		if (UWorld* World = PlayerCharacter->GetWorld())
		{
			const float DebugDuration = EquippedFirearm->GetDebugHitscanRayDuration();
			const float DebugThickness = EquippedFirearm->GetDebugHitscanRayThickness();

			if (const ABOShotgunFirearm* ShotgunFirearm = Cast<ABOShotgunFirearm>(EquippedFirearm))
			{
				const TArray<FVector> PelletDirections = ShotgunFirearm->BuildPelletDirections(FireDirection);
				const float PelletTraceDistance = ShotgunFirearm->GetPelletTraceDistance();

				for (const FVector& PelletDirection : PelletDirections)
				{
					const FVector PredictedTraceEnd = MuzzleLocation + PelletDirection.GetSafeNormal() * PelletTraceDistance;
					DrawPredictedFireDebugLine(
						World,
						MuzzleLocation,
						PredictedTraceEnd,
						EquippedFirearm->GetOwner(),
						EquippedFirearm,
						DebugDuration,
						DebugThickness);
				}
			}
			else
			{
				const FVector PredictedTraceEnd = MuzzleLocation + FireDirection.GetSafeNormal() * PredictedFireDebugTraceDistance;
				DrawPredictedFireDebugLine(
					World,
					MuzzleLocation,
					PredictedTraceEnd,
					EquippedFirearm->GetOwner(),
					EquippedFirearm,
					DebugDuration,
					DebugThickness);
			}
		}
	}

	// 로컬 예측은 체감용 연출만 담당하고, 실제 월드 판정은 서버에서만 수행합니다.
	if (PlayerCharacter->HasAuthority())
	{
		float CurrentClipAmmoForLog = -1.0f;
		if (const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			const FGameplayTag EquippedWeaponSlotTag = CombatComponent->GetEquippedWeaponSlotTag();
			CurrentClipAmmoForLog = EquippedWeaponSlotTag == BlackoutGameplayTags::Weapon_Secondary
				? AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute())
				: AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute());
		}

		BO_LOG_GAS(Log,
			"GA_FireWeapon authoritative shot: Character=%s Weapon=%s ClipAmmo=%.0f Time=%.3f Automatic=%s Shotgun=%s",
			*GetNameSafe(PlayerCharacter),
			*GetNameSafe(EquippedFirearm),
			CurrentClipAmmoForLog,
			PlayerCharacter->GetWorld() ? PlayerCharacter->GetWorld()->GetTimeSeconds() : -1.0f,
			EquippedFirearm->IsAutomatic() ? TEXT("true") : TEXT("false"),
			Cast<ABOShotgunFirearm>(EquippedFirearm) ? TEXT("true") : TEXT("false"));

		if (ABOShotgunFirearm* ShotgunFirearm = Cast<ABOShotgunFirearm>(EquippedFirearm))
		{
			const FGameplayEffectSpecHandle PelletDamageSpecHandle = BuildPelletDamageSpec(ShotgunFirearm);
			ShotgunFirearm->FireShotgun(FireDirection, PelletDamageSpecHandle);
		}
		else
		{
			const FGameplayEffectSpecHandle DamageSpecHandle = BuildDamageSpec(EquippedFirearm);
			EquippedFirearm->Fire(FireDirection, DamageSpecHandle);
		}

		// 실제 월드에 영향을 주는 큐는 서버에서만 실행합니다.
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Weapon_Fire);
		}
	}
	else
	{
		BO_LOG_GAS(Verbose, "GA_FireWeapon predicted client: 실제 사격 판정은 서버에서 대기");
	}

	// 4. 탄퍼짐 누적 및 반동 적용
	CombatComponent->OnShotFired();

	if (CachedFireMontage)
	{
		if (PlayerCharacter->HasAuthority())
		{
			const float MontageDuration = CachedFireMontage->GetPlayLength();
			if (MontageDuration > 0.0f)
			{
				if (UWorld* World = ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->GetWorld() : nullptr)
				{
					World->GetTimerManager().SetTimer(FireMontageCompletionTimerHandle, this, &UBlackoutGA_FireWeapon::OnFireMontageCompleted, MontageDuration, false);
				}
				return;
			}

			OnFireMontageCompleted();
		}

		return;
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UBlackoutGA_FireWeapon::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get());
		UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
		ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;

		if (bWasCancelled && PlayerCharacter && CachedFireMontage)
		{
			if (PlayerCharacter->IsLocallyControlled())
			{
				PlayerCharacter->StopFireMontage(CachedFireMontage, 0.1f);
			}

			if (PlayerCharacter->HasAuthority())
			{
				PlayerCharacter->Multicast_StopFireMontage(CachedFireMontage, 0.1f);
			}
		}

		if (bWasCancelled && bWeaponFireAnimationTriggered && EquippedFirearm)
		{
			if (PlayerCharacter->IsLocallyControlled())
			{
				EquippedFirearm->StopWeaponFireAnimation();
			}

			if (PlayerCharacter->HasAuthority())
			{
				EquippedFirearm->Multicast_StopWeaponFireAnimation();
			}
		}

		if (UWorld* World = ActorInfo->AvatarActor->GetWorld())
		{
			World->GetTimerManager().ClearTimer(FireMontageCompletionTimerHandle);
		}
	}

	CachedFireMontage = nullptr;
	bWeaponFireAnimationTriggered = false;
	BO_LOG_GAS(Log, "GA_FireWeapon ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FHitResult UBlackoutGA_FireWeapon::PerformTrace(const FVector& Start, const FVector& End)
{
	FHitResult HitResult;
	// TODO: 무기 유효 사거리 및 카메라-총구 간 크로스헤어 보정을 포함한 라인 트레이스 구현
	return HitResult;
}

FGameplayEffectSpecHandle UBlackoutGA_FireWeapon::BuildDamageSpec(const ABOFirearm* Firearm)
{
	FGameplayEffectSpecHandle SpecHandle;
	if (!DamageEffectClass)
	{
		BO_LOG_GAS(Error, "BuildDamageSpec failed: DamageEffectClass가 설정되지 않음 (Ability=%s)", *GetNameSafe(this));
		return SpecHandle;
	}

	if (Firearm && GetAbilitySystemComponentFromActorInfo())
	{
		SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, Firearm->GetBaseDamage());
		}
	}
	else
	{
		BO_LOG_GAS(Warning,
		           "BuildDamageSpec failed: Firearm=%s ASC=%s",
		           *GetNameSafe(Firearm),
		           *GetNameSafe(GetAbilitySystemComponentFromActorInfo()));
	}
	return SpecHandle;
}

FGameplayEffectSpecHandle UBlackoutGA_FireWeapon::BuildPelletDamageSpec(const ABOShotgunFirearm* Firearm)
{
	FGameplayEffectSpecHandle SpecHandle;
	if (!DamageEffectClass)
	{
		BO_LOG_GAS(Error, "BuildPelletDamageSpec failed: DamageEffectClass가 설정되지 않음 (Ability=%s)", *GetNameSafe(this));
		return SpecHandle;
	}

	if (Firearm && GetAbilitySystemComponentFromActorInfo())
	{
		SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_Damage, Firearm->GetDamagePerPellet());
		}
	}
	else
	{
		BO_LOG_GAS(Warning,
		           "BuildPelletDamageSpec failed: Firearm=%s ASC=%s",
		           *GetNameSafe(Firearm),
		           *GetNameSafe(GetAbilitySystemComponentFromActorInfo()));
	}
	return SpecHandle;
}

bool UBlackoutGA_FireWeapon::ApplyAmmoCost()
{
	const ABlackoutPlayerCharacter* PlayerCharacter = CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	const UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	if (!CombatComponent || !AbilitySystemComponent)
	{
		return false;
	}

	const FGameplayTag WeaponSlotTag = CombatComponent->GetEquippedWeaponSlotTag();
	const float PrimaryClipAmmoBefore = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute());
	const float SecondaryClipAmmoBefore = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute());
	BO_LOG_GAS(Log,
		"ApplyAmmoCost before: Character=%s Slot=%s Primary=%.0f Secondary=%.0f Local=%s Authority=%s",
		*GetNameSafe(PlayerCharacter),
		*WeaponSlotTag.ToString(),
		PrimaryClipAmmoBefore,
		SecondaryClipAmmoBefore,
		PlayerCharacter && PlayerCharacter->IsLocallyControlled() ? TEXT("true") : TEXT("false"),
		PlayerCharacter && PlayerCharacter->HasAuthority() ? TEXT("true") : TEXT("false"));

	if (WeaponSlotTag == BlackoutGameplayTags::Weapon_Secondary)
	{
		const float SecondaryClipAmmo = SecondaryClipAmmoBefore;
		if (SecondaryClipAmmo < 1.0f)
		{
			BO_LOG_GAS(Warning,
				"ApplyAmmoCost failed: 보조 무기 탄약 부족 Character=%s Secondary=%.0f",
				*GetNameSafe(PlayerCharacter),
				SecondaryClipAmmo);
			return false;
		}

		AbilitySystemComponent->ApplyModToAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute(), EGameplayModOp::Additive, -1.0f);
		BO_LOG_GAS(Log,
			"ApplyAmmoCost after: Character=%s Slot=%s Primary=%.0f Secondary=%.0f",
			*GetNameSafe(PlayerCharacter),
			*WeaponSlotTag.ToString(),
			AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute()),
			AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute()));
		return true;
	}

	const float PrimaryClipAmmo = PrimaryClipAmmoBefore;
	if (PrimaryClipAmmo < 1.0f)
	{
		BO_LOG_GAS(Warning,
			"ApplyAmmoCost failed: 주 무기 탄약 부족 Character=%s Primary=%.0f",
			*GetNameSafe(PlayerCharacter),
			PrimaryClipAmmo);
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute(), EGameplayModOp::Additive, -1.0f);
	BO_LOG_GAS(Log,
		"ApplyAmmoCost after: Character=%s Slot=%s Primary=%.0f Secondary=%.0f",
		*GetNameSafe(PlayerCharacter),
		*WeaponSlotTag.ToString(),
		AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute()),
		AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute()));
	return true;
}

bool UBlackoutGA_FireWeapon::CanFireAtCurrentTime(ABOFirearm* Firearm, const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!Firearm || LastFireRateGateWeapon.Get() != Firearm)
	{
		return true;
	}

	const UWorld* World = ActorInfo && ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->GetWorld() : nullptr;
	if (!World)
	{
		return true;
	}

	return World->GetTimeSeconds() + KINDA_SMALL_NUMBER >= NextAllowedFireTimeSeconds;
}

void UBlackoutGA_FireWeapon::ReserveNextFireTime(ABOFirearm* Firearm, const FGameplayAbilityActorInfo* ActorInfo)
{
	if (!Firearm)
	{
		return;
	}

	const UWorld* World = ActorInfo && ActorInfo->AvatarActor.IsValid() ? ActorInfo->AvatarActor->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	LastFireRateGateWeapon = Firearm;

	const float FireRate = Firearm->GetFireRate();
	if (FireRate <= 0.0f)
	{
		NextAllowedFireTimeSeconds = World->GetTimeSeconds();
		return;
	}

	NextAllowedFireTimeSeconds = World->GetTimeSeconds() + (1.0f / FireRate);
}

void UBlackoutGA_FireWeapon::PlayFireMontage()
{
	ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
	if (!PlayerCharacter || !EquippedFirearm)
	{
		BO_LOG_GAS(Warning, "PlayFireMontage failed: PlayerCharacter 또는 EquippedFirearm이 유효하지 않음");
		return;
	}

	UAnimMontage* FireMontage = CachedFireMontage.Get();
	if (!FireMontage)
	{
		return;
	}

	if (PlayerCharacter->IsLocallyControlled())
	{
		PlayerCharacter->PlayFireMontage(FireMontage, 1.f, false);
	}

	if (PlayerCharacter->HasAuthority())
	{
		PlayerCharacter->Multicast_PlayFireMontage(FireMontage, 1.f, false);
	}

	BO_LOG_GAS(Log,
		"PlayFireMontage: Character=%s Weapon=%s Montage=%s",
		*GetNameSafe(PlayerCharacter),
		*GetNameSafe(EquippedFirearm),
		*GetNameSafe(FireMontage));
}

void UBlackoutGA_FireWeapon::OnWeaponFireStartEventReceived(FGameplayEventData Payload)
{
	if (bWeaponFireAnimationTriggered)
	{
		return;
	}

	ABlackoutPlayerCharacter* PlayerCharacter =
		CurrentActorInfo ? Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
	UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
	ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
	if (!PlayerCharacter || !EquippedFirearm)
	{
		BO_LOG_GAS(Warning, "GA_FireWeapon fire event ignored: PlayerCharacter 또는 EquippedFirearm이 유효하지 않음");
		return;
	}

	bWeaponFireAnimationTriggered = true;

	if (PlayerCharacter->IsLocallyControlled())
	{
		EquippedFirearm->PlayWeaponFireAnimation();
	}

	if (PlayerCharacter->HasAuthority())
	{
		EquippedFirearm->Multicast_PlayWeaponFireAnimation();
	}

	BO_LOG_GAS(Log,
		"GA_FireWeapon fire event received: Character=%s Weapon=%s EventTag=%s",
		*GetNameSafe(PlayerCharacter),
		*GetNameSafe(EquippedFirearm),
		*Payload.EventTag.ToString());
}

void UBlackoutGA_FireWeapon::OnFireMontageCompleted()
{
	if (!CurrentActorInfo || !CurrentActorInfo->AvatarActor.IsValid() || !CurrentActorInfo->AvatarActor->HasAuthority())
	{
		return;
	}

	if (CachedFireMontage && !bWeaponFireAnimationTriggered)
	{
		ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
		UBlackoutCombatComponent* CombatComponent = PlayerCharacter ? PlayerCharacter->GetCombatComponent() : nullptr;
		const ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent->GetEquippedFirearm() : nullptr;
		BO_LOG_GAS(Warning,
			"GA_FireWeapon montage completed without fire event: Character=%s Weapon=%s Montage=%s",
			*GetNameSafe(PlayerCharacter),
			*GetNameSafe(EquippedFirearm),
			*GetNameSafe(CachedFireMontage));
	}

	BO_LOG_GAS(Log, "GA_FireWeapon montage completed");
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
