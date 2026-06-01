#include "Animation/BlackoutPlayerAnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
#include "Combat/Weapons/BOFirearm.h"
#include "Combat/Weapons/BOWeaponBase.h"
#include "Core/BlackoutCollisionChannels.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"

void UBlackoutPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<ABlackoutPlayerCharacter>(OwnerCharacter);
}

void UBlackoutPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<ABlackoutPlayerCharacter>(OwnerCharacter);
		if (!PlayerCharacter) return;
	}

	// GAS 태그 상태 업데이트
	bIsAiming = false;
	bIsTwoHanded = false;
	bIsSprinting = false;

	const UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent();
	if (CombatComponent)
	{
		bIsAiming = CombatComponent->IsAiming();
		if (const ABOFirearm* EquippedFirearm = CombatComponent->GetEquippedFirearm())
		{
			bIsTwoHanded = EquippedFirearm->UsesTwoHandedAnimation();
		}
	}

	UpdateLeftHandIK(CombatComponent);

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	{
		bIsSprinting = ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Sprinting);

		if (!PlayerCharacter->GetCombatComponent())
		{
			bIsAiming = ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Aiming);
		}
	}
	
	UpdateAimOffset(DeltaSeconds);
}

void UBlackoutPlayerAnimInstance::UpdateAimOffset(float DeltaSeconds)
{
	if (!PlayerCharacter)
	{
		return;
	}

	const UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent();
	if (!bIsAiming || !CombatComponent || !CombatComponent->GetEquippedFirearm())
	{
		ResetAimOffset();
		ReplicateAimOffset(DeltaSeconds);
		return;
	}

	if (!PlayerCharacter->IsLocallyControlled())
	{
		ApplyReplicatedAimOffset(DeltaSeconds);
		return;
	}

	UpdateAimTarget();

	const FVector AimOrigin = CombatComponent->GetMuzzleTransform().GetLocation();

	// 뷰 방향 벡터 조회
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	if (!GetAimTraceViewPoint(ViewLocation, ViewRotation))
	{
		ResetAimOffset();
		ReplicateAimOffset(DeltaSeconds);
		return;
	}

	const FVector ViewDir = ViewRotation.Vector();

	// 부호화 투영 거리(내적)로 근거리 파랄랙스가 커지는 구간을 판정합니다.
	const float ProjectedDistance = FVector::DotProduct(AimTargetLocation - AimOrigin, ViewDir);

	const float TargetViewBlendAlpha = BlackoutAimOffsetMath::CalculateEyeBlendAlpha(ProjectedDistance, AimOffsetBlendSettings);
	CurrentAimOffsetViewBlendAlpha = BlackoutAimOffsetMath::InterpEyeBlendAlpha(
		CurrentAimOffsetViewBlendAlpha,
		TargetViewBlendAlpha,
		DeltaSeconds,
		AimOffsetBlendSettings);

	const FRotator MuzzleAimRotation = UKismetMathLibrary::FindLookAtRotation(AimOrigin, AimTargetLocation);
	FRotator ViewTargetAimRotation = ViewRotation;
	const FVector EyeLocation = PlayerCharacter->GetPawnViewLocation();
	if ((AimTargetLocation - EyeLocation).SizeSquared() > KINDA_SMALL_NUMBER)
	{
		// ViewRotation 자체가 아니라 눈 위치에서 카메라 타겟을 바라보는 각도를 사용해 근거리 파랄랙스를 반영합니다.
		ViewTargetAimRotation = UKismetMathLibrary::FindLookAtRotation(EyeLocation, AimTargetLocation);
	}

	const FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	const FRotator MuzzleDelta = UKismetMathLibrary::NormalizedDeltaRotator(MuzzleAimRotation, ActorRotation);
	const FRotator ViewDelta = UKismetMathLibrary::NormalizedDeltaRotator(ViewTargetAimRotation, ActorRotation);

	const float MuzzleYaw = FMath::Clamp(MuzzleDelta.Yaw, -180.f, 180.f);
	const float MuzzlePitch = FMath::Clamp(MuzzleDelta.Pitch, -90.f, 90.f);
	const float ViewYaw = FMath::Clamp(ViewDelta.Yaw, -180.f, 180.f);
	const float ViewPitch = FMath::Clamp(ViewDelta.Pitch, -90.f, 90.f);

	float TargetYaw = BlackoutAimOffsetMath::BlendAngleDegrees(MuzzleYaw, ViewYaw, CurrentAimOffsetViewBlendAlpha);
	float TargetPitch = BlackoutAimOffsetMath::BlendAngleDegrees(MuzzlePitch, ViewPitch, CurrentAimOffsetViewBlendAlpha);
	TargetYaw = FMath::Clamp(FRotator::NormalizeAxis(TargetYaw), -180.f, 180.f);
	TargetPitch = FMath::Clamp(FRotator::NormalizeAxis(TargetPitch), -90.f, 90.f);

	if (FMath::IsNearlyEqual(CurrentAimOffsetViewBlendAlpha, 1.f, KINDA_SMALL_NUMBER))
	{
		TargetYaw = ViewYaw;
		TargetPitch = ViewPitch;
	}

	AO_Yaw = FMath::FInterpTo(AO_Yaw, TargetYaw, DeltaSeconds, AO_InterpSpeed);
	AO_Pitch = FMath::FInterpTo(AO_Pitch, TargetPitch, DeltaSeconds, AO_InterpSpeed);

	ReplicateAimOffset(DeltaSeconds);
}

void UBlackoutPlayerAnimInstance::ResetAimOffset()
{
	AO_Yaw = 0.f;
	AO_Pitch = 0.f;
	AimTargetLocation = FVector::ZeroVector;
	AimTargetActor = nullptr;
	bHasAimTarget = false;
	CurrentAimOffsetViewBlendAlpha = 0.f;
}

void UBlackoutPlayerAnimInstance::ApplyReplicatedAimOffset(float DeltaSeconds)
{
	const FVector2D ReplicatedAimOffset = PlayerCharacter ? PlayerCharacter->GetReplicatedAimOffset() : FVector2D::ZeroVector;
	AO_Yaw = FMath::FInterpTo(AO_Yaw, ReplicatedAimOffset.X, DeltaSeconds, AO_InterpSpeed);
	AO_Pitch = FMath::FInterpTo(AO_Pitch, ReplicatedAimOffset.Y, DeltaSeconds, AO_InterpSpeed);
}

void UBlackoutPlayerAnimInstance::ReplicateAimOffset(float DeltaSeconds)
{
	if (!PlayerCharacter || !PlayerCharacter->IsLocallyControlled() || PlayerCharacter->HasAuthority())
	{
		return;
	}

	AimOffsetReplicationElapsed += DeltaSeconds;
	const FVector2D CurrentAimOffset(AO_Yaw, AO_Pitch);
	if (AimOffsetReplicationElapsed < AimOffsetReplicationInterval)
	{
		return;
	}

	const bool bAimOffsetChanged = (CurrentAimOffset - LastReplicatedAimOffset).SizeSquared() >= FMath::Square(AimOffsetReplicationTolerance);
	if (!bAimOffsetChanged)
	{
		AimOffsetReplicationElapsed = 0.f;
		return;
	}

	AimOffsetReplicationElapsed = 0.f;
	LastReplicatedAimOffset = CurrentAimOffset;
	PlayerCharacter->Server_SetAimOffset(CurrentAimOffset);
}

void UBlackoutPlayerAnimInstance::UpdateLeftHandIK(const UBlackoutCombatComponent* CombatComponent)
{
	bHasLeftHandIKTarget = false;
	LeftHandIKLocation = FVector::ZeroVector;
	LeftHandIKRotation = FRotator::ZeroRotator;

	if (!PlayerCharacter || !CombatComponent)
	{
		return;
	}

	const ABOWeaponBase* EquippedWeapon = CombatComponent->GetEquippedWeapon();
	USkeletalMeshComponent* CharacterMesh = PlayerCharacter->GetMesh();
	if (!EquippedWeapon || !EquippedWeapon->HasLeftHandIKTarget() || !CharacterMesh)
	{
		return;
	}

	if (CharacterMesh->GetBoneIndex(LeftHandIKReferenceBoneName) == INDEX_NONE)
	{
		return;
	}

	const FTransform LeftHandIKWorldTransform = EquippedWeapon->GetLeftHandIKTransform();
	FRotator UnusedBoneSpaceRotation = FRotator::ZeroRotator;
	CharacterMesh->TransformToBoneSpace(
		LeftHandIKReferenceBoneName,
		LeftHandIKWorldTransform.GetLocation(),
		LeftHandIKWorldTransform.Rotator(),
		LeftHandIKLocation,
		UnusedBoneSpaceRotation);

	// ModifyBone의 Component Space Replace 회전에 바로 사용할 수 있도록 무기 소켓 회전을 변환합니다.
	LeftHandIKRotation = CharacterMesh->GetComponentTransform()
		.InverseTransformRotation(LeftHandIKWorldTransform.GetRotation())
		.Rotator();

	bHasLeftHandIKTarget = true;
}

void UBlackoutPlayerAnimInstance::UpdateAimTarget()
{
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	if (!GetAimTraceViewPoint(ViewLocation, ViewRotation))
	{
		AimTargetActor = nullptr;
		bHasAimTarget = false;
		AimTargetLocation = PlayerCharacter->GetPawnViewLocation() + PlayerCharacter->GetControlRotation().Vector() * AimTraceDistance;
		return;
	}

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * AimTraceDistance;
	AimTargetLocation = TraceEnd;
	AimTargetActor = nullptr;
	bHasAimTarget = false;

	UWorld* World = PlayerCharacter->GetWorld();
	if (!World)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(BlackoutPlayerAnimInstance_UpdateAimTarget), false, PlayerCharacter);
	if (const UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		if (ABOWeaponBase* EquippedWeapon = CombatComponent->GetEquippedWeapon())
		{
			QueryParams.AddIgnoredActor(EquippedWeapon);
		}
	}

	FHitResult HitResult;
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, BlackoutCollisionChannels::WeaponTrace, QueryParams)
		&& HitResult.bBlockingHit)
	{
		AimTargetLocation = HitResult.ImpactPoint;
		AimTargetActor = HitResult.GetActor();
		bHasAimTarget = true;
	}
}

bool UBlackoutPlayerAnimInstance::GetAimTraceViewPoint(FVector& OutViewLocation, FRotator& OutViewRotation) const
{
	if (!PlayerCharacter)
	{
		return false;
	}

	if (AController* Controller = PlayerCharacter->GetController())
	{
		Controller->GetPlayerViewPoint(OutViewLocation, OutViewRotation);
		return true;
	}

	OutViewLocation = PlayerCharacter->GetPawnViewLocation();
	OutViewRotation = PlayerCharacter->GetControlRotation();
	return true;
}
