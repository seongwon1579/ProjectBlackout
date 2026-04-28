#include "Animation/BlackoutPlayerAnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
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
	bIsTwoHanded = false;
	bHasLeftHandIKTarget = false;
	LeftHandIKLocation = FVector::ZeroVector;
	LeftHandIKRotation = FRotator::ZeroRotator;

	if (const UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		bIsAiming = CombatComponent->IsAiming();
		const FGameplayTag EquippedWeaponSlotTag = CombatComponent->GetEquippedWeaponSlotTag();
		bIsTwoHanded = EquippedWeaponSlotTag == BlackoutGameplayTags::Weapon_Primary;

		const ABOWeaponBase* EquippedWeapon = CombatComponent->GetEquippedWeapon();
		USkeletalMeshComponent* CharacterMesh = PlayerCharacter->GetMesh();
		const bool bWeaponHasLeftHandIKTarget = EquippedWeapon && EquippedWeapon->HasLeftHandIKTarget();

		if (bWeaponHasLeftHandIKTarget && CharacterMesh)
		{
			const FTransform LeftHandIKWorldTransform = EquippedWeapon->GetLeftHandIKTransform();
			if (CharacterMesh->GetBoneIndex(LeftHandIKReferenceBoneName) != INDEX_NONE)
			{
				const FTransform ReferenceBoneWorldTransform = CharacterMesh->GetSocketTransform(LeftHandIKReferenceBoneName, RTS_World);
				CharacterMesh->TransformToBoneSpace(
					LeftHandIKReferenceBoneName,
					LeftHandIKWorldTransform.GetLocation(),
					LeftHandIKWorldTransform.Rotator(),
					LeftHandIKLocation,
					LeftHandIKRotation);

				const FQuat LeftHandIKRelativeRotation = ReferenceBoneWorldTransform.GetRotation().Inverse() * LeftHandIKWorldTransform.GetRotation();
				LeftHandIKRotation = LeftHandIKRelativeRotation.Rotator();

				bHasLeftHandIKTarget = true;
			}
		}
	}

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

	if (!bIsAiming)
	{
		ResetAimOffset();
		return;
	}

	UpdateAimTarget();

	const FRotator AimRotation = UKismetMathLibrary::FindLookAtRotation(PlayerCharacter->GetPawnViewLocation(), AimTargetLocation);
	const FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRotation);
	const float TargetYaw = FMath::Clamp(Delta.Yaw, -180.f, 180.f);
	const float TargetPitch = FMath::Clamp(Delta.Pitch, -90.f, 90.f);

	AO_Yaw = FMath::FInterpTo(AO_Yaw, TargetYaw, DeltaSeconds, AO_InterpSpeed);
	AO_Pitch = FMath::FInterpTo(AO_Pitch, TargetPitch, DeltaSeconds, AO_InterpSpeed);
}

void UBlackoutPlayerAnimInstance::ResetAimOffset()
{
	AO_Yaw = 0.f;
	AO_Pitch = 0.f;
	AimTargetLocation = FVector::ZeroVector;
	AimTargetActor = nullptr;
	bHasAimTarget = false;
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
