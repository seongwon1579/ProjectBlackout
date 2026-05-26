#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_SummonMinion.h"

#include "BlackoutBossCharacter.h"
#include "BlackoutGameplayTags.h"
#include "BOEnemySpawnerProjectile.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/GameplayStatics.h"

void UBlackoutGA_Ravager_SummonMinion::SetupEventListeners()
{
	Super::SetupEventListeners();
	
	if (WaitSpawnEvent)
	{
		WaitSpawnEvent->EndTask();
		WaitSpawnEvent = nullptr;
	}
	
	WaitSpawnEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BlackoutGameplayTags::Event_Enemy_Attack_Spawn_Projectile,
		nullptr,
		true,
		false);
	
	if (WaitSpawnEvent)
	{
		WaitSpawnEvent->EventReceived.AddDynamic(this, &UBlackoutGA_Ravager_SummonMinion::OnSpawnMinionNotify);
		WaitSpawnEvent->ReadyForActivation();
	}
}

bool UBlackoutGA_Ravager_SummonMinion::HasValidSettings() const
{
	return CachedPatternData->MinionSettings.IsValid();
}

void UBlackoutGA_Ravager_SummonMinion::OnSpawnMinionNotify(FGameplayEventData Payload)
{
	if (!CanActivatePattern()) return;
	
	SetSpawnerProjectiles();
}

void UBlackoutGA_Ravager_SummonMinion::SetSpawnerProjectiles()
{
	if (!CanActivatePattern()) return;
	
	const FBossMinionSpawnSettings& Settings = CachedPatternData->MinionSettings;
	
	FVector SpawnLocation;
	ResolveSpawnLocation(SpawnLocation);
	
	const FRotator BaseRotation = CachedOwner->GetActorRotation();
	const int32 Count = Settings.SpawnCount;
	
	for (int32 i = 0; i < Count; i++)
	{
		ThrowSingleSpawnerProjectile(SpawnLocation, BaseRotation, i , Count);
	}
}

void UBlackoutGA_Ravager_SummonMinion::ThrowSingleSpawnerProjectile(const FVector& SpawnLocation, const FRotator& BaseRotation,
	int32 Index, int32 Total)
{
	if (!CanActivatePattern()) return;
	
	const FBossMinionSpawnSettings& Settings = CachedPatternData->MinionSettings;
	
	float YawOffset = 0.f;
	if (Total > 1)
	{
		const float Step = (Settings.SpreadAngle * 2.f) / (Total - 1);
		YawOffset = -Settings.SpreadAngle + Step * Index;
	}
	FRotator ThrowRotation = BaseRotation;
	ThrowRotation.Yaw += YawOffset;
	ThrowRotation.Pitch = Settings.ThrowPitch;
	
	UWorld* World = GetWorld();
	if (!World) return;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CachedOwner;
	SpawnParams.Instigator = CachedOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	ABOEnemySpawnerProjectile* SpawnerProjectile = World->SpawnActor<ABOEnemySpawnerProjectile>(
		Settings.SpawnerClass,
		SpawnLocation,
		ThrowRotation,
		SpawnParams
		);
	if (SpawnerProjectile)
	{
		SpawnerProjectile->InitializeProjectile(Settings.ProjectileSpawnData);
		SpawnerProjectile->SetSpawnerData(Settings.MinionSpawnData);
	}
}

void UBlackoutGA_Ravager_SummonMinion::ResolveSpawnLocation(FVector& OutLocation) const
{
	if (!CanActivatePattern()) return;
	
	const FName& SocketName = CachedPatternData->MinionSettings.SocketName;
	USkeletalMeshComponent* Mesh = CachedOwner->GetMesh();
	
	if (Mesh && Mesh->DoesSocketExist(SocketName))
	{
		OutLocation = Mesh->GetSocketLocation(SocketName);
	}
	else
	{
		OutLocation = CachedOwner->GetActorLocation() + FVector(0, 0, 100);
		UE_LOG(LogTemp, Warning, TEXT("[%s] Socket '%s' not found"), *GetName(), *SocketName.ToString());
	}
}
