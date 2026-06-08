// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/UBlackoutRavagerAnimInstance.h"

#include "KismetAnimationLibrary.h"
#include "GameFramework/Character.h"

void UUBlackoutRavagerAnimInstance::OnDeath()
{
	IsDead = true;
}

void UUBlackoutRavagerAnimInstance::UpdateAnimationProperties()
{
	ACharacter* Char = Cast<ACharacter>(TryGetPawnOwner());
	if (!Char) return;
	
	const FVector Vel = Char->GetVelocity();
	Speed = Vel.Size2D();
	Direction = UKismetAnimationLibrary::CalculateDirection(Vel, Char->GetActorRotation());
	AimDirection = 0;
}
