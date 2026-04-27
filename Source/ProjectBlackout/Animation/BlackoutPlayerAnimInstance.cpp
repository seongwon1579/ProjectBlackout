#include "Animation/BlackoutPlayerAnimInstance.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "Combat/Components/BlackoutCombatComponent.h"
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

	if (const UBlackoutCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent())
	{
		bIsAiming = CombatComponent->IsAiming();
		bIsTwoHanded = CombatComponent->GetEquippedWeaponSlotTag() == BlackoutGameplayTags::Weapon_Primary;
	}

	if (UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent())
	{
		bIsSprinting = ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Sprinting);

		if (!PlayerCharacter->GetCombatComponent())
		{
			bIsAiming = ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Aiming);
		}
	}

	// 에임 오프셋 계산 (조준 시 상체 회전을 위함)
	// 컨트롤 회전(카메라)과 캐릭터 정면 사이의 차이 계산
	const FRotator ControlRotation = PlayerCharacter->GetControlRotation();
	const FRotator ActorRotation = PlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);

	// 부드러운 애니메이션을 위해 보간(Interpolation) 적용
	AO_Yaw = FMath::FInterpTo(AO_Yaw, Delta.Yaw, DeltaSeconds, AO_InterpSpeed);
	AO_Pitch = FMath::FInterpTo(AO_Pitch, Delta.Pitch, DeltaSeconds, AO_InterpSpeed);
}
