#include "Animation/BlackoutAnimInstanceBase.h"
#include "Characters/BlackoutCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"

void UBlackoutAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ABlackoutCharacterBase>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UBlackoutAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter || !OwnerMovementComponent)
	{
		// 초기화 실패 시 재시도
		OwnerCharacter = Cast<ABlackoutCharacterBase>(TryGetPawnOwner());
		if (OwnerCharacter)
		{
			OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
		}
		
		if (!OwnerCharacter || !OwnerMovementComponent) return;
	}

	// 기본 이동 데이터 업데이트
	Velocity = OwnerCharacter->GetVelocity();
	GroundSpeed = Velocity.Size2D();

	// 이동 방향 계산 (-180 ~ 180)
	MovementDirection = CalculateDirection(Velocity, OwnerCharacter->GetActorRotation());

	bIsFalling = OwnerMovementComponent->IsFalling();
	
	// 가속도 벡터의 크기가 0보다 크면 가속 중(입력 중)인 것으로 판단
	bIsAccelerating = OwnerMovementComponent->GetCurrentAcceleration().SizeSquared() > 0.f;

	// GAS 태그 상태 업데이트
	if (UAbilitySystemComponent* ASC = OwnerCharacter->GetAbilitySystemComponent())
	{
		bIsDowned = ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed);
		bIsLocked = ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Locked);
	}
}
