#include "GAS/Tasks/AbilityTask_BossMeleeHitbox.h"
#include "Abilities/GameplayAbility.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"

UAbilityTask_BossMeleeHitbox* UAbilityTask_BossMeleeHitbox::InitCustomTask(
	UGameplayAbility* OwningAbility,
	UPrimitiveComponent* InHitbox)
{
	UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: Init"))
	UAbilityTask_BossMeleeHitbox* Task = NewAbilityTask<UAbilityTask_BossMeleeHitbox>(OwningAbility);
	Task->Hitbox = InHitbox;
	return Task;
}

void UAbilityTask_BossMeleeHitbox::Activate()
{
	Super::Activate();
	
	bTickingTask = true;

	HitboxComp = Hitbox.Get();
	UE_LOG(LogTemp, Warning, TEXT("Collision Profile: %s"),
	*HitboxComp->GetCollisionProfileName().ToString());
	
	UE_LOG(LogTemp, Warning, TEXT("Hitbox Component Name: %s"),
	*HitboxComp->GetName());
	
	if (!HitboxComp) EndTask();
	
	HitActors.Empty();
	
	// 오버랩 이벤트 등록
	HitboxComp->OnComponentBeginOverlap.AddDynamic(this, &UAbilityTask_BossMeleeHitbox::OnHitboxBeginOverlap);
	
	// On Collision
	HitboxComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HitboxComp->SetGenerateOverlapEvents(true);
	
	
	UE_LOG(LogTemp, Warning, TEXT("Collision Enabled After Set: %d"),
	(int32)HitboxComp->GetCollisionEnabled());
	
	UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: On Hit Box"))
}

void UAbilityTask_BossMeleeHitbox::OnDestroy(bool bInOwnerFinished)
{
	UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: OnDestroy"))
	
	// if (UPrimitiveComponent* HitboxComp = Hitbox.Get())
	// {
	// 	HitboxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 	HitboxComp->OnComponentBeginOverlap.RemoveDynamic(this, &UAbilityTask_BossMeleeHitbox::OnHitboxBeginOverlap);
	// }
	HitboxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitboxComp->OnComponentBeginOverlap.RemoveDynamic(this, &UAbilityTask_BossMeleeHitbox::OnHitboxBeginOverlap);

	HitActors.Empty();
	Super::OnDestroy(bInOwnerFinished);
}

void UAbilityTask_BossMeleeHitbox::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	
	// if (USphereComponent* Sphere = Cast<USphereComponent>(HitboxComp))
	// {
	// 	DrawDebugSphere(
	// 		GetWorld(),
	// 		Sphere->GetComponentLocation(),
	// 		Sphere->GetScaledSphereRadius(),
	// 		16,
	// 		FColor::Green,
	// 		false,
	// 		3.f,
	// 		0,
	// 		2.f
	// 	);
	// }
	//
	// if (!HitboxComp) return;
	//
	// const FVector Loc = HitboxComp->GetComponentLocation();
	// const FQuat Rot = HitboxComp->GetComponentQuat();
	// const float Lifetime = 2.f; 
	//
	// if (USphereComponent* Sphere = Cast<USphereComponent>(HitboxComp))
	// {
	// 	DrawDebugSphere(GetWorld(), Loc, Sphere->GetScaledSphereRadius(),
	// 		16, FColor::Green, false, Lifetime, 0, 1.5f);
	// }
	// else if (UBoxComponent* Box = Cast<UBoxComponent>(HitboxComp))
	// {
	// 	DrawDebugBox(GetWorld(), Loc, Box->GetScaledBoxExtent(), Rot,
	// 		FColor::Green, false, Lifetime, 0, 1.5f);
	// }
	// else if (UCapsuleComponent* Cap = Cast<UCapsuleComponent>(HitboxComp))
	// {
	// 	DrawDebugCapsule(GetWorld(), Loc,
	// 		Cap->GetScaledCapsuleHalfHeight(), Cap->GetScaledCapsuleRadius(),
	// 		Rot, FColor::Green, false, Lifetime, 0, 1.5f);
	// }
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
	UE_LOG(LogTemp, Warning, TEXT("UAbilityTask_BossMeleeHitbox: HitHit"))
	
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
