#include "Animation/BlackoutBossAnimInstance.h"

#include "Characters/BlackoutBossCharacter.h"
#include "ProjectBlackoutCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UBlackoutBossAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BossCharacter = Cast<ABlackoutBossCharacter>(TryGetPawnOwner());
	
}

void UBlackoutBossAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	ACharacter* Char = Cast<ACharacter>(TryGetPawnOwner());
	if (!Char) return;
	
	const FVector Vel = Char->GetVelocity();
	Speed = Vel.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Vel, Char->GetActorRotation());
	AimDirection = 0;
}


void UBlackoutBossAnimInstance::OnTurnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsTurning = false;

	APawn* Pawn = TryGetPawnOwner();
	if (ACharacter* Char = Cast<ACharacter>(Pawn))
	{
		Char->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}