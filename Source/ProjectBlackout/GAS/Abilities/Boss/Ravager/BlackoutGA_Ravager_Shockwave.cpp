#include "GAS/Abilities/Boss/Ravager/GA_Ravager_Shockwave.h"

#include "BlackoutBossCharacter.h"
#include "BlackoutGameplayTags.h"
#include "BOEnemyProjectile.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

void UGA_Ravager_Shockwave::PreActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo,
                                               const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	Super::PreActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	TrySetupMotionWarp(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Ravager_Shockwave::SetupEventListeners()
{
	Super::SetupEventListeners();

	WaitSpawnEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_Spawn_Projectile,
		nullptr,
		true,
		false
	);
	if (WaitSpawnEvent)
	{
		WaitSpawnEvent->EventReceived.AddDynamic(this, &UGA_Ravager_Shockwave::OnSpawnProjectileNotify);
		WaitSpawnEvent->ReadyForActivation();
	}
}

bool UGA_Ravager_Shockwave::HasValidSettings() const
{
	return CachedPatternData->ProjectileSettings.IsValid();
}

void UGA_Ravager_Shockwave::OnSpawnProjectileNotify(FGameplayEventData Payload)
{
	if (!CanActivatePattern()) return;

	const TSubclassOf<ABOEnemyProjectile> ProjectileClass = CachedPatternData->ProjectileSettings.ProjectileClass;

	FVector SpawnLocation;
	FRotator SpawnRotation;
	ResolveSpawnTransform(SpawnLocation, SpawnRotation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CachedOwner;
	SpawnParams.Instigator = CachedOwner;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UWorld* World = GetWorld();
	if (!World) return;

	ABOEnemyProjectile* Projectile = World->SpawnActor<ABOEnemyProjectile>(
		ProjectileClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams);
	
	if (Projectile)
	{
		Projectile->InitializeProjectile(CachedPatternData->ProjectileSettings.ProjectileSpawnData);
	}
}

void UGA_Ravager_Shockwave::ResolveSpawnTransform(FVector& OutLocation, FRotator& OutRotation) const
{
	if (!CanActivatePattern())
	{
		OutLocation = FVector::ZeroVector;
		OutRotation = FRotator::ZeroRotator;
		return;
	}

	OutRotation = CachedOwner->GetActorRotation();

	USkeletalMeshComponent* Mesh = CachedOwner->GetMesh();
	if (Mesh && Mesh->DoesSocketExist(CachedPatternData->ProjectileSettings.SocketName))
	{
		OutLocation = Mesh->GetSocketLocation(CachedPatternData->ProjectileSettings.SocketName);
	}
	else
	{
		OutLocation = CachedOwner->GetActorLocation();
		UE_LOG(LogTemp, Warning, TEXT("Does not exit SocketName in Projectile Object"));
	}
}
