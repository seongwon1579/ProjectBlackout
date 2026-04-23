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

	AProjectBlackoutCharacter* TestCharacter = Cast<AProjectBlackoutCharacter>(TryGetPawnOwner());
	if (!TestCharacter) return;

	UCharacterMovementComponent* MoveComp = TestCharacter->GetCharacterMovement();
	if (!MoveComp) return;

	const FVector Vel = TestCharacter->GetVelocity();
	Speed = Vel.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Vel, TestCharacter->GetActorRotation());
	
}

void UBlackoutBossAnimInstance::StartTurn(float AngleDelta)
{
	// APawn* Pawn = TryGetPawnOwner();
	// if (!Pawn || bIsTurning) return;
	// if (FMath::Abs(AngleDelta) < 10.f) return;
	//
	// bIsTurning = true;
	//
	// ACharacter* Char = Cast<ACharacter>(Pawn);
	// Char->GetCharacterMovement()->bOrientRotationToMovement = false;
	//
	// UAnimMontage* MontageToPlay = (AngleDelta < 0.f) ? TurnLeftMontage : TurnRightMontage;
	// Montage_Play(MontageToPlay, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
	//
	// // 몽타주 끝나면 콜백
	// FOnMontageEnded EndDelegate;
	// EndDelegate.BindUObject(this, &UBlackoutBossAnimInstance::OnTurnMontageEnded);
	// Montage_SetEndDelegate(EndDelegate, MontageToPlay);
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