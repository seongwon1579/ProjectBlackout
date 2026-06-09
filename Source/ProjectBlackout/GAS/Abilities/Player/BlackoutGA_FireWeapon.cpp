#include "GAS/Abilities/Player/BlackoutGA_FireWeapon.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlackoutPlayerAnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Combat/BlackoutWeaponCueLibrary.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Components/BlackoutHitboxComponent.h"
#include "Combat/Components/BlackoutImpactIndicatorComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOShotgunFirearm.h"
#include "Combat/Weapons/BOWeaponDebugUtils.h"
#include "Core/BlackoutAimOffsetTypes.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Core/BlackoutLog.h"
#include "DrawDebugHelpers.h"
#include "Interfaces/BlackoutDamageable.h"
#include "GameplayCueManager.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "GAS/Attributes/BlackoutAmmoAttributeSet.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Framework/BlackoutPlayerState.h"
#include "UI/BlackoutHUD.h"

namespace
{
	constexpr float PredictedFireDebugTraceDistance = 10000.0f;

	const ABlackoutPlayerState* ResolveOwningBlackoutPlayerState(const FGameplayAbilityActorInfo* ActorInfo)
	{
		if (!ActorInfo)
		{
			return nullptr;
		}

		if (const ABlackoutPlayerState* PlayerState = Cast<ABlackoutPlayerState>(ActorInfo->OwnerActor.Get()))
		{
			return PlayerState;
		}

		if (const ABlackoutPlayerCharacter* PlayerCharacter = Cast<ABlackoutPlayerCharacter>(ActorInfo->AvatarActor.Get()))
		{
			return PlayerCharacter->GetPlayerState<ABlackoutPlayerState>();
		}

		return nullptr;
	}

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

	bool TryShowPredictedDamageNumber(
		ABlackoutPlayerCharacter* PlayerCharacter,
		const FHitResult& PredictedHitResult,
		const float PredictedDamageAmount)
	{
		if (!PlayerCharacter || !PlayerCharacter->IsLocallyControlled() || !PredictedHitResult.bBlockingHit)
		{
			return false;
		}

		bool bCanShowDamageNumber = false;
		bool bIsCritical = false;
		float PredictedDisplayDamageAmount = PredictedDamageAmount;

		if (UBlackoutHitboxComponent* HitboxComponent = Cast<UBlackoutHitboxComponent>(PredictedHitResult.GetComponent()))
		{
			bCanShowDamageNumber = HitboxComponent->GetOwner() && Cast<IBlackoutDamageable>(HitboxComponent->GetOwner());
			bIsCritical = HitboxComponent->GetPartTag().MatchesTagExact(BlackoutGameplayTags::Body_WeakSpot);
			PredictedDisplayDamageAmount *= HitboxComponent->GetDamageMultiplier();
		}
		else if (AActor* HitActor = PredictedHitResult.GetActor())
		{
			if (IBlackoutDamageable* Damageable = Cast<IBlackoutDamageable>(HitActor))
			{
				bCanShowDamageNumber = true;
				bIsCritical = Damageable->GetHitPartTag(PredictedHitResult.BoneName).MatchesTagExact(BlackoutGameplayTags::Body_WeakSpot);
			}
		}

		if (!bCanShowDamageNumber)
		{
			return false;
		}

		APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
		ABlackoutHUD* BlackoutHUD = PlayerController ? Cast<ABlackoutHUD>(PlayerController->GetHUD()) : nullptr;
		if (!BlackoutHUD)
		{
			return false;
		}

		return BlackoutHUD->ShowDamageNumberAtWorldLocation(
			PredictedDisplayDamageAmount,
			PredictedHitResult.ImpactPoint,
			bIsCritical);
	}

	void ShowPredictedDamageNumberFromTrace(
		ABlackoutPlayerCharacter* PlayerCharacter,
		UWorld* World,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		const AActor* IgnoredOwner,
		const AActor* IgnoredWeapon,
		const float PredictedDamageAmount)
	{
		if (!World || !PlayerCharacter || !PlayerCharacter->IsLocallyControlled())
		{
			return;
		}

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutGA_FireWeapon_PredictedDamageNumber), false, IgnoredOwner);
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

		TryShowPredictedDamageNumber(PlayerCharacter, PredictedHitResult, PredictedDamageAmount);
	}

	void BuildPredictedCueHitResult(
		UWorld* World,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		const AActor* IgnoredOwner,
		const AActor* IgnoredWeapon,
		FHitResult& OutHitResult)
	{
		OutHitResult = FHitResult();
		OutHitResult.TraceStart = TraceStart;
		OutHitResult.TraceEnd = TraceEnd;

		if (!World)
		{
			return;
		}

		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutGA_FireWeapon_PredictedCue), false, IgnoredOwner);
		QueryParams.bReturnPhysicalMaterial = true;
		if (IgnoredWeapon)
		{
			QueryParams.AddIgnoredActor(IgnoredWeapon);
		}

		World->LineTraceSingleByChannel(
			OutHitResult,
			TraceStart,
			TraceEnd,
			BlackoutCollisionChannels::WeaponTrace,
			QueryParams);

		OutHitResult.TraceStart = TraceStart;
		OutHitResult.TraceEnd = TraceEnd;
	}

	void ExecuteLocalWeaponCue(AActor* CueTarget, const FGameplayTag& CueTag, const FGameplayCueParameters& CueParameters)
	{
		if (!CueTarget || !CueTag.IsValid())
		{
			return;
		}

		if (UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager())
		{
			CueManager->HandleGameplayCue(CueTarget, CueTag, EGameplayCueEvent::Executed, CueParameters);
			return;
		}

		BO_LOG_GAS(Error, "ExecuteLocalWeaponCue failed: GameplayCueManager가 유효하지 않음 (Cue=%s)", *CueTag.ToString());
	}

	void ExecuteLocalTrailCue(
		ABlackoutPlayerCharacter* CueTarget,
		const FBlackoutWeaponCueSet& CueSet,
		AActor* SourceActor,
		const FVector& MuzzleLocation,
		const FHitResult& HitResult,
		const FVector& TraceEnd)
	{
		ExecuteLocalWeaponCue(
			CueTarget,
			CueSet.TrailCueTag,
			UBlackoutWeaponCueLibrary::BuildTrailCueParameters(SourceActor, MuzzleLocation, HitResult, TraceEnd));
	}

	void ExecuteLocalImpactCue(
		ABlackoutPlayerCharacter* CueTarget,
		const FBlackoutWeaponCueSet& CueSet,
		AActor* SourceActor,
		const FHitResult& HitResult)
	{
		if (!HitResult.bBlockingHit)
		{
			return;
		}

		const FGameplayTag SurfaceTag = UBlackoutWeaponCueLibrary::ResolveSurfaceTag(HitResult);
		const FGameplayTag ImpactCueTag = UBlackoutWeaponCueLibrary::ResolveImpactCueTag(CueSet, SurfaceTag);
		ExecuteLocalWeaponCue(CueTarget, ImpactCueTag, UBlackoutWeaponCueLibrary::BuildImpactCueParameters(SourceActor, HitResult));
	}

	void AddTrailCueEntry(
		TArray<FBlackoutWeaponGameplayCueEntry>& CueEntries,
		const FBlackoutWeaponCueSet& CueSet,
		AActor* SourceActor,
		const FVector& MuzzleLocation,
		const FHitResult& HitResult,
		const FVector& TraceEnd)
	{
		if (!CueSet.TrailCueTag.IsValid())
		{
			return;
		}

		FBlackoutWeaponGameplayCueEntry CueEntry;
		CueEntry.CueTag = CueSet.TrailCueTag;
		CueEntry.CueParameters = UBlackoutWeaponCueLibrary::BuildTrailCueParameters(SourceActor, MuzzleLocation, HitResult, TraceEnd);
		CueEntries.Add(CueEntry);
	}

	void AddImpactCueEntry(
		TArray<FBlackoutWeaponGameplayCueEntry>& CueEntries,
		const FBlackoutWeaponCueSet& CueSet,
		AActor* SourceActor,
		const FHitResult& HitResult)
	{
		if (!HitResult.bBlockingHit)
		{
			return;
		}

		const FGameplayTag SurfaceTag = UBlackoutWeaponCueLibrary::ResolveSurfaceTag(HitResult);
		const FGameplayTag ImpactCueTag = UBlackoutWeaponCueLibrary::ResolveImpactCueTag(CueSet, SurfaceTag);
		if (!ImpactCueTag.IsValid())
		{
			return;
		}

		FBlackoutWeaponGameplayCueEntry CueEntry;
		CueEntry.CueTag = ImpactCueTag;
		CueEntry.CueParameters = UBlackoutWeaponCueLibrary::BuildImpactCueParameters(SourceActor, HitResult);
		CueEntries.Add(CueEntry);
	}

	void ExecutePredictedWeaponCues(
		UAbilitySystemComponent* AbilitySystemComponent,
		ABlackoutPlayerCharacter* PlayerCharacter,
		ABOFirearm* EquippedFirearm,
		const FVector& MuzzleLocation,
		const FVector& FireDirection,
		const float FallbackTraceDistance)
	{
		if (!AbilitySystemComponent || !PlayerCharacter || !PlayerCharacter->IsLocallyControlled() || PlayerCharacter->HasAuthority() || !EquippedFirearm)
		{
			return;
		}

		const FBlackoutWeaponCueSet WeaponCueSet = EquippedFirearm->GetWeaponCueSet();
		UBlackoutWeaponCueLibrary::ExecuteFireCue(AbilitySystemComponent, WeaponCueSet, EquippedFirearm, MuzzleLocation, FireDirection);

		UWorld* World = PlayerCharacter->GetWorld();
		const AActor* IgnoredOwner = EquippedFirearm->GetOwner();

		if (const ABOShotgunFirearm* ShotgunFirearm = Cast<ABOShotgunFirearm>(EquippedFirearm))
		{
			const TArray<FVector> PelletDirections = ShotgunFirearm->BuildPelletDirections(FireDirection);
			const float PelletTraceDistance = ShotgunFirearm->GetPelletTraceDistance();

			for (const FVector& PelletDirection : PelletDirections)
			{
				const FVector PelletTraceEnd = MuzzleLocation + PelletDirection.GetSafeNormal() * PelletTraceDistance;
				FHitResult PredictedHitResult;
				BuildPredictedCueHitResult(World, MuzzleLocation, PelletTraceEnd, IgnoredOwner, EquippedFirearm, PredictedHitResult);

				// 산탄은 같은 Cue 태그를 한 프레임에 여러 번 실행하므로 ASC의 중복 실행 병합을 우회합니다.
				ExecuteLocalTrailCue(PlayerCharacter, WeaponCueSet, EquippedFirearm, MuzzleLocation, PredictedHitResult, PelletTraceEnd);
				ExecuteLocalImpactCue(PlayerCharacter, WeaponCueSet, EquippedFirearm, PredictedHitResult);
			}

			return;
		}

		const FVector TraceEnd = MuzzleLocation + FireDirection.GetSafeNormal() * FallbackTraceDistance;
		FHitResult PredictedHitResult;
		if (EquippedFirearm->UsesHitscan())
		{
			BuildPredictedCueHitResult(World, MuzzleLocation, TraceEnd, IgnoredOwner, EquippedFirearm, PredictedHitResult);
		}
		else
		{
			PredictedHitResult.TraceStart = MuzzleLocation;
			PredictedHitResult.TraceEnd = TraceEnd;
		}

		ExecuteLocalTrailCue(PlayerCharacter, WeaponCueSet, EquippedFirearm, MuzzleLocation, PredictedHitResult, TraceEnd);
		ExecuteLocalImpactCue(PlayerCharacter, WeaponCueSet, EquippedFirearm, PredictedHitResult);
	}

	void MulticastTrailCueToRemoteProxies(
		ABlackoutPlayerCharacter* CueOwner,
		const FBlackoutWeaponCueSet& CueSet,
		AActor* SourceActor,
		const FVector& MuzzleLocation,
		const FHitResult& HitResult,
		const FVector& TraceEnd)
	{
		if (!CueOwner || !CueSet.TrailCueTag.IsValid())
		{
			return;
		}

		CueOwner->Multicast_ExecuteWeaponGameplayCue(
			CueSet.TrailCueTag,
			UBlackoutWeaponCueLibrary::BuildTrailCueParameters(SourceActor, MuzzleLocation, HitResult, TraceEnd),
			true);
	}

	void MulticastImpactCueToRemoteProxies(
		ABlackoutPlayerCharacter* CueOwner,
		const FBlackoutWeaponCueSet& CueSet,
		AActor* SourceActor,
		const FHitResult& HitResult)
	{
		if (!CueOwner || !HitResult.bBlockingHit)
		{
			return;
		}

		const FGameplayTag SurfaceTag = UBlackoutWeaponCueLibrary::ResolveSurfaceTag(HitResult);
		const FGameplayTag ImpactCueTag = UBlackoutWeaponCueLibrary::ResolveImpactCueTag(CueSet, SurfaceTag);
		if (!ImpactCueTag.IsValid())
		{
			return;
		}

		CueOwner->Multicast_ExecuteWeaponGameplayCue(
			ImpactCueTag,
			UBlackoutWeaponCueLibrary::BuildImpactCueParameters(SourceActor, HitResult),
			true);
	}
}

UBlackoutGA_FireWeapon::UBlackoutGA_FireWeapon()
{
	InputID = EBlackoutAbilityInputID::Fire;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CancelAbilitiesWithTag.AddTag(BlackoutGameplayTags::Ability_Player_Dodge);
	CancelAbilitiesWithTag.AddTag(BlackoutGameplayTags::Ability_Player_Melee);

	ActivationRequiredTags.AddTag(BlackoutGameplayTags::State_Aiming);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Sprinting);
	ActivationBlockedTags.AddTag(BlackoutGameplayTags::State_Reloading);
}

void UBlackoutGA_FireWeapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	BO_LOG_GAS(Verbose, "GA_FireWeapon activate requested");

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

	const FGameplayTag FireAnimTag = EquippedFirearm->GetFireAnimTag();
	// 시카고 타자기는 CombatComponent 자동사격 타이머를 발사 간격의 기준으로 사용합니다.
	const bool bBypassFireRateGate = FireAnimTag.MatchesTagExact(BlackoutGameplayTags::Animation_Fire_ChicagoTypewriter);

	// 단발/반자동 무기는 몽타주 유무와 무관하게 발사 간격을 강제로 보장합니다.
	if (!bBypassFireRateGate && !CanFireAtCurrentTime(EquippedFirearm, ActorInfo))
	{
		const float CurrentTimeSeconds = ActorInfo && ActorInfo->AvatarActor.IsValid() && ActorInfo->AvatarActor->GetWorld()
			? ActorInfo->AvatarActor->GetWorld()->GetTimeSeconds()
			: -1.0f;
		BO_LOG_GAS(Verbose,
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
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!bBypassFireRateGate)
	{
		ReserveNextFireTime(EquippedFirearm, ActorInfo);
	}

	BO_LOG_GAS(Verbose, "GA_FireWeapon activated: Character=%s, Weapon=%s", *GetNameSafe(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr), *GetNameSafe(EquippedFirearm));

	CachedFireMontage = PlayerCharacter ? PlayerCharacter->GetFireMontageForTag(FireAnimTag) : nullptr;
	const bool bShouldWaitForFireMontageCompletion =
		CachedFireMontage
		&& !FireAnimTag.MatchesTagExact(BlackoutGameplayTags::Animation_Fire_ChicagoTypewriter);
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
	FVector AimTarget = ImpactIndicatorComponent->GetAimTargetPoint();
	const FVector CameraAimTarget = AimTarget;

	// 사격 판정(LineTrace) 궤적을 애니메이션 방향과 동기화
	float Threshold = 10.f;
	float BlendRange = 15.f;
	float FallbackDist = 100.f;
	FBlackoutAimOffsetBlendSettings AimOffsetBlendSettings;

	if (UBlackoutPlayerAnimInstance* AnimInstance = Cast<UBlackoutPlayerAnimInstance>(PlayerCharacter->GetMesh()->GetAnimInstance()))
	{
		Threshold = AnimInstance->GetSafeAimDistanceThreshold();
		BlendRange = AnimInstance->GetSafeAimBlendRange();
		FallbackDist = AnimInstance->GetFallbackAimTargetDistance();
		AimOffsetBlendSettings = AnimInstance->GetAimOffsetBlendSettings();
	}

	// 뷰 방향 벡터 조회
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	APlayerController* PlayerController = Cast<APlayerController>(PlayerCharacter->GetController());
	if (PlayerController)
	{
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	const FVector ViewDir = ViewRotation.Vector();

	// 부호화 투영 거리(내적) 계산
	const float ProjectedDistance = FVector::DotProduct(AimTarget - MuzzleLocation, ViewDir);

	// Alpha 계산: ProjectedDistance가 Threshold 이하(음수 포함)이면 1.0(완전 폴백), Threshold + BlendRange 이상이면 0.0(완전 기본)
	float BlendAlpha = 0.f;
	if (ProjectedDistance <= Threshold)
	{
		BlendAlpha = 1.0f;
	}
	else if (ProjectedDistance >= Threshold + BlendRange)
	{
		BlendAlpha = 0.0f;
	}
	else if (BlendRange > 0.f)
	{
		BlendAlpha = 1.0f - ((ProjectedDistance - Threshold) / BlendRange);
	}

	if (BlendAlpha > 0.f && !ViewDir.IsNearlyZero())
	{
		const FVector FallbackTarget = ViewLocation + ViewDir * FallbackDist;
		AimTarget = AimTarget + (FallbackTarget - AimTarget) * BlendAlpha;
	}

	const float EyeDirectionAlpha = BlackoutAimOffsetMath::CalculateEyeBlendAlpha(ProjectedDistance, AimOffsetBlendSettings);

	FVector BaseFireDirection = (AimTarget - MuzzleLocation).GetSafeNormal();
	if (EyeDirectionAlpha > 0.f && !ViewDir.IsNearlyZero())
	{
		FVector EyeTargetDirection = ViewDir.GetSafeNormal();
		const FVector EyeLocation = PlayerCharacter->GetPawnViewLocation();
		const FVector EyeToTarget = CameraAimTarget - EyeLocation;
		if (!EyeToTarget.IsNearlyZero())
		{
			// 근거리에서는 카메라 방향 자체가 아니라 눈 위치에서 카메라 타겟을 바라보는 방향으로 탄도를 맞춥니다.
			EyeTargetDirection = EyeToTarget.GetSafeNormal();
		}

		BaseFireDirection = BlackoutAimOffsetMath::BlendDirection(BaseFireDirection, EyeTargetDirection, EyeDirectionAlpha);
	}

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

	// 로컬 예측 숫자는 정확한 서버 계산 대신 즉시 피드백을 우선합니다.
	if (PlayerCharacter->IsLocallyControlled() && EquippedFirearm->UsesHitscan())
	{
		if (UWorld* World = PlayerCharacter->GetWorld())
		{
			if (const ABOShotgunFirearm* ShotgunFirearm = Cast<ABOShotgunFirearm>(EquippedFirearm))
			{
				const TArray<FVector> PelletDirections = ShotgunFirearm->BuildPelletDirections(FireDirection);
				const float PelletTraceDistance = ShotgunFirearm->GetPelletTraceDistance();
				const float PredictedPelletDamage = ShotgunFirearm->GetDamagePerPellet();

				for (const FVector& PelletDirection : PelletDirections)
				{
					const FVector PredictedTraceEnd = MuzzleLocation + PelletDirection.GetSafeNormal() * PelletTraceDistance;
					ShowPredictedDamageNumberFromTrace(
						PlayerCharacter,
						World,
						MuzzleLocation,
						PredictedTraceEnd,
						EquippedFirearm->GetOwner(),
						EquippedFirearm,
						PredictedPelletDamage);
				}
			}
			else
			{
				const FVector PredictedTraceEnd = MuzzleLocation + FireDirection.GetSafeNormal() * PredictedFireDebugTraceDistance;
				ShowPredictedDamageNumberFromTrace(
					PlayerCharacter,
					World,
					MuzzleLocation,
					PredictedTraceEnd,
					EquippedFirearm->GetOwner(),
					EquippedFirearm,
					EquippedFirearm->GetBaseDamage());
			}
		}
	}

	// 예측 클라이언트는 발사 입력 직후 로컬 GCN을 실행하고, 서버 복제본은 예측키로 중복 재생을 억제합니다.
	ExecutePredictedWeaponCues(
		GetAbilitySystemComponentFromActorInfo(),
		PlayerCharacter,
		EquippedFirearm,
		MuzzleLocation,
		FireDirection,
		ParallaxMaxDistance);

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

		BO_LOG_GAS(Verbose,
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
			const FBlackoutWeaponCueSet WeaponCueSet = ShotgunFirearm->GetWeaponCueSet();
			UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
			UBlackoutWeaponCueLibrary::ExecuteFireCue(AbilitySystemComponent, WeaponCueSet, ShotgunFirearm, MuzzleLocation, FireDirection);

			const TArray<FBlackoutShotgunPelletHit> PelletHits = ShotgunFirearm->FireShotgun(FireDirection, PelletDamageSpecHandle);
			TArray<FBlackoutWeaponGameplayCueEntry> ShotgunCueEntries;
			ShotgunCueEntries.Reserve(PelletHits.Num() * 2);

			for (const FBlackoutShotgunPelletHit& PelletHit : PelletHits)
			{
				const FVector PelletTraceEnd = PelletHit.HitResult.TraceEnd.IsNearlyZero()
					? MuzzleLocation + FireDirection.GetSafeNormal() * ShotgunFirearm->GetPelletTraceDistance()
					: FVector(PelletHit.HitResult.TraceEnd);

				AddTrailCueEntry(ShotgunCueEntries, WeaponCueSet, ShotgunFirearm, MuzzleLocation, PelletHit.HitResult, PelletTraceEnd);
				AddImpactCueEntry(ShotgunCueEntries, WeaponCueSet, ShotgunFirearm, PelletHit.HitResult);
			}

			if (!ShotgunCueEntries.IsEmpty())
			{
				PlayerCharacter->Multicast_ExecuteWeaponGameplayCueBatch(ShotgunCueEntries, true);
			}
		}
		else
		{
			const FGameplayEffectSpecHandle DamageSpecHandle = BuildDamageSpec(EquippedFirearm);
			const FBlackoutWeaponCueSet WeaponCueSet = EquippedFirearm->GetWeaponCueSet();
			UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
			UBlackoutWeaponCueLibrary::ExecuteFireCue(AbilitySystemComponent, WeaponCueSet, EquippedFirearm, MuzzleLocation, FireDirection);

			const FHitResult ShotHitResult = EquippedFirearm->Fire(FireDirection, DamageSpecHandle);
			const FVector TraceEnd = ShotHitResult.TraceEnd.IsNearlyZero()
				? MuzzleLocation + FireDirection.GetSafeNormal() * ParallaxMaxDistance
				: FVector(ShotHitResult.TraceEnd);

			if (EquippedFirearm->UsesHitscan())
			{
				MulticastTrailCueToRemoteProxies(
					PlayerCharacter,
					WeaponCueSet,
					EquippedFirearm,
					MuzzleLocation,
					ShotHitResult,
					TraceEnd);
				MulticastImpactCueToRemoteProxies(PlayerCharacter, WeaponCueSet, EquippedFirearm, ShotHitResult);
			}
			else
			{
				FHitResult ProjectileTrailHit;
				ProjectileTrailHit.TraceStart = MuzzleLocation;
				ProjectileTrailHit.TraceEnd = TraceEnd;
				MulticastTrailCueToRemoteProxies(
					PlayerCharacter,
					WeaponCueSet,
					EquippedFirearm,
					MuzzleLocation,
					ProjectileTrailHit,
					TraceEnd);
			}
		}
	}
	else
	{
		BO_LOG_GAS(Verbose, "GA_FireWeapon predicted client: 실제 사격 판정은 서버에서 대기");
	}

	// 4. 탄퍼짐 누적 및 반동 적용
	CombatComponent->OnShotFired();

	if (bShouldWaitForFireMontageCompletion)
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
		ABOFirearm* EquippedFirearm = CombatComponent ? CombatComponent ->GetEquippedFirearm() : nullptr;

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
	BO_LOG_GAS(Verbose, "GA_FireWeapon ended: Cancelled=%s", bWasCancelled ? TEXT("true") : TEXT("false"));
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
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_DamageNumber_PredictedOnly, 1.0f);
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
			SpecHandle.Data->SetSetByCallerMagnitude(BlackoutGameplayTags::Data_DamageNumber_PredictedOnly, 1.0f);
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

	if (const ABlackoutPlayerState* BlackoutPlayerState = ResolveOwningBlackoutPlayerState(CurrentActorInfo))
	{
		if (BlackoutPlayerState->HasInfiniteAmmoCheat())
		{
			return true;
		}
	}

	const FGameplayTag WeaponSlotTag = CombatComponent->GetEquippedWeaponSlotTag();
	const float PrimaryClipAmmoBefore = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute());
	const float SecondaryClipAmmoBefore = AbilitySystemComponent->GetNumericAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute());

	BO_LOG_GAS(Verbose,
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
			BO_LOG_GAS(Verbose,
				"ApplyAmmoCost failed: 보조 무기 탄약 부족 Character=%s Secondary=%.0f",
				*GetNameSafe(PlayerCharacter),
				SecondaryClipAmmo);
			return false;
		}

		AbilitySystemComponent->ApplyModToAttribute(UBlackoutAmmoAttributeSet::GetSecondaryClipAmmoAttribute(), EGameplayModOp::Additive, -1.0f);
		BO_LOG_GAS(Verbose,
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
		BO_LOG_GAS(Verbose,
			"ApplyAmmoCost failed: 주 무기 탄약 부족 Character=%s Primary=%.0f",
			*GetNameSafe(PlayerCharacter),
			PrimaryClipAmmo);
		return false;
	}

	AbilitySystemComponent->ApplyModToAttribute(UBlackoutAmmoAttributeSet::GetPrimaryClipAmmoAttribute(), EGameplayModOp::Additive, -1.0f);
	BO_LOG_GAS(Verbose,
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

	BO_LOG_GAS(Verbose,
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

	BO_LOG_GAS(Verbose,
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

	BO_LOG_GAS(Verbose, "GA_FireWeapon montage completed");
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
