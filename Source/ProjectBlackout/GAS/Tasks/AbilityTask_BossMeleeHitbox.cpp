#include "GAS/Tasks/AbilityTask_BossMeleeHitbox.h"
#include "Abilities/GameplayAbility.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"
#include "DrawDebugHelpers.h"
#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_Base.h"

UAbilityTask_BossMeleeHitbox* UAbilityTask_BossMeleeHitbox::InitCustomTask(
	UGameplayAbility* OwningAbility,
	UPrimitiveComponent* InHitbox)
{
	UAbilityTask_BossMeleeHitbox* Task = NewAbilityTask<UAbilityTask_BossMeleeHitbox>(OwningAbility);
	Task->Hitbox = InHitbox;
	if (const UBlackoutGA_Ravager_Base* RavagerAbility = Cast<UBlackoutGA_Ravager_Base>(OwningAbility))
	{
		Task->bEnableBossDebug = RavagerAbility->IsBossDebugEnabled();
	}
	if (Task->bEnableBossDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: Init"))
	}
	return Task;
}

void UAbilityTask_BossMeleeHitbox::Activate()
{
	Super::Activate();
	
	bTickingTask = true;

	HitboxComp = Hitbox.Get();
	if (!HitboxComp)
	{
		EndTask();
		return;
	}

	if (bEnableBossDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Collision Profile: %s"),
		*HitboxComp->GetCollisionProfileName().ToString());
	
		UE_LOG(LogTemp, Warning, TEXT("Hitbox Component Name: %s"),
		*HitboxComp->GetName());
	}
	
	HitActors.Empty();
	
	// 오버랩 이벤트 등록
	HitboxComp->OnComponentBeginOverlap.AddDynamic(this, &UAbilityTask_BossMeleeHitbox::OnHitboxBeginOverlap);
	
	// On Collision
	HitboxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HitboxComp->SetGenerateOverlapEvents(true);
	
	
	if (bEnableBossDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("Collision Enabled After Set: %d"),
		(int32)HitboxComp->GetCollisionEnabled());
	
		UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: On Hit Box"))
	}
}

void UAbilityTask_BossMeleeHitbox::OnDestroy(bool bInOwnerFinished)
{
	if (bEnableBossDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: OnDestroy"))
	}
	
	if (HitboxComp)
	{
		HitboxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitboxComp->OnComponentBeginOverlap.RemoveDynamic(this, &UAbilityTask_BossMeleeHitbox::OnHitboxBeginOverlap);
	}

	HitActors.Empty();
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_BossMeleeHitbox::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!bEnableBossDebug)
	{
		return;
	}
	
#if ENABLE_DRAW_DEBUG
	if (USphereComponent* Sphere = Cast<USphereComponent>(HitboxComp))
	{
		DrawDebugSphere(
			GetWorld(),
			Sphere->GetComponentLocation(),
			Sphere->GetScaledSphereRadius(),
			16,
			FColor::Green,
			false,
			3.f,
			0,
			2.f
		);
	}
	
	if (!HitboxComp) return;

	const FVector Loc = HitboxComp->GetComponentLocation();
	const FQuat Rot = HitboxComp->GetComponentQuat();
	const float Lifetime = 2.f; 

	if (USphereComponent* Sphere = Cast<USphereComponent>(HitboxComp))
	{
		DrawDebugSphere(GetWorld(), Loc, Sphere->GetScaledSphereRadius(),
			16, FColor::Green, false, Lifetime, 0, 1.5f);
	}
	else if (UBoxComponent* Box = Cast<UBoxComponent>(HitboxComp))
	{
		DrawDebugBox(GetWorld(), Loc, Box->GetScaledBoxExtent(), Rot,
			FColor::Green, false, Lifetime, 0, 1.5f);
	}
	else if (UCapsuleComponent* Cap = Cast<UCapsuleComponent>(HitboxComp))
	{
		DrawDebugCapsule(GetWorld(), Loc,
			Cap->GetScaledCapsuleHalfHeight(), Cap->GetScaledCapsuleRadius(),
			Rot, FColor::Green, false, Lifetime, 0, 1.5f);
	}
#endif
}

void UAbilityTask_BossMeleeHitbox::OnHitboxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& HitResult)
{
	if (!OtherActor) return;
	
	AActor* SelfActor = Ability ? Ability->GetAvatarActorFromActorInfo() : nullptr;
	if (OtherActor == SelfActor) return;

	for (const TWeakObjectPtr<AActor>& Already : HitActors)
	{
		if (Already.Get() == OtherActor) return;
	}

	HitActors.Add(OtherActor);
	if (bEnableBossDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: HitHit"))
	}
	
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		FHitResult Result = HitResult;
		if (!Result.GetActor())
		{
			Result.HitObjectHandle = FActorInstanceHandle(OtherActor);
		}
		OnHit.Broadcast(Result);
	}
}
