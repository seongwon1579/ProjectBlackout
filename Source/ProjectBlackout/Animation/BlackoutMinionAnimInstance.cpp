// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/BlackoutMinionAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/Character.h"

// void UBlackoutMinionAnimInstance::OnDeath()
// {
// 	bIsDead = true;
// }

void UBlackoutMinionAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UBlackoutMinionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	ACharacter* Char = Cast<ACharacter>(TryGetPawnOwner());
	if (!Char) return;
	
	const FVector Vel = Char->GetVelocity();
	Speed = Vel.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Vel, Char->GetActorRotation());
}
