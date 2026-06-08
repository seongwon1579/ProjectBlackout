#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_SummonMinion.h"

#include "BlackoutBossCharacter.h"
#include "BlackoutGameplayTags.h"
#include "BOEnemySpawnerProjectile.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "NavigationSystem.h"
#include "BlackoutLog.h"
#include "BlackoutRavagerAIController.h"
#include "BORavagerBoss.h"
#include "GameFramework/Character.h"

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
	if (!CanActivatePattern())
	{
		UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] CanActivatePattern failed in SetSpawnerProjectiles."));
		return;
	}
	
	const FBossMinionSpawnSettings& Settings = CachedPatternData->MinionSettings;
	
	FVector SpawnLocation;
	ResolveSpawnLocation(SpawnLocation);
	
	const FRotator BaseRotation = CachedOwner->GetActorRotation();
	const int32 Count = Settings.SpawnCount;
	
	for (int32 i = 0; i < Count; i++)
	{
		ThrowSingleSpawnerProjectile(SpawnLocation, BaseRotation, i , Count);
	}

	// 보스 페이즈 판정 및 엘리트 미니언 즉시 스폰 (Phase 2 이상일 때)
	if (ABlackoutRavagerAIController* BossAIC = Cast<ABlackoutRavagerAIController>(CachedOwner->GetController()))
	{
		if (BossAIC->GetCurrentPhase() >= EBOBossPhase::Phase2)
		{
			SpawnEliteMinionsDirectly();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] Failed to get BlackoutBossAIController from Owner controller."));
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
		const float Step       = (Settings.SpreadAngle * 2.f) / Total;    
		const float CellCenter = -Settings.SpreadAngle + Step * (Index + 0.5f); 
		YawOffset = CellCenter + FMath::FRandRange(-Step * 0.5f, Step * 0.5f);  
	}
	else
	{
		YawOffset = FMath::FRandRange(-Settings.SpreadAngle, Settings.SpreadAngle);
	}

	FRotator ThrowRotation = BaseRotation;
	ThrowRotation.Yaw   += YawOffset;
	ThrowRotation.Pitch  = Settings.ThrowPitch + FMath::FRandRange(-Settings.ThrowPitchVariance, Settings.ThrowPitchVariance);

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner    = CachedOwner;
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
		FProjectileSpawnData PerShotData = Settings.ProjectileSpawnData;
		PerShotData.Speed *= FMath::FRandRange(Settings.DistanceScaleMin, Settings.DistanceScaleMax);

		SpawnerProjectile->InitializeProjectile(PerShotData);
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

void UBlackoutGA_Ravager_SummonMinion::SpawnEliteMinionsDirectly()
{
	if (!CachedOwner || !CachedPatternData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] CachedOwner or CachedPatternData is null inside SpawnEliteMinionsDirectly!"));
		return;
	}

	const FBossMinionSpawnSettings& Settings = CachedPatternData->MinionSettings;
	if (!Settings.EliteMinionSpawnData.MinionClass)
	{
		BO_LOG_AI(Warning, "[SummonMinion] EliteMinionSpawnData.MinionClass is null. Please allocate it in the BORavagerPatternData asset.");
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		BO_LOG_AI(Warning, "[SummonMinion] World is null inside SpawnEliteMinionsDirectly.");
		return;
	}

	const int32 EliteCount = Settings.EliteSpawnCount;
	const float SpawnRadius = Settings.EliteSpawnRadius;
	const FVector BossLocation = CachedOwner->GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CachedOwner;
	SpawnParams.Instigator = CachedOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < EliteCount; i++)
	{
		// 보스 몸체와 직접 겹치지 않도록 최소 200.f ~ 최대 SpawnRadius 사이에서 거리를 무작위 선정
		const float RandomAngle = FMath::FRand() * 360.f;
		const float RandomRadius = FMath::FRandRange(200.f, FMath::Max(200.f, SpawnRadius));

		FVector SpawnLocation = BossLocation + FRotator(0.f, RandomAngle, 0.f).Vector() * RandomRadius;
		SpawnLocation.Z = BossLocation.Z + 50.f; // 지면에 배치하기 위한 오프셋 적용

		// NavMesh 투영을 통해 장애물 및 벽 내부에 끼는 것 방지
		if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			FNavLocation ProjectedLocation;
			if (NavSys->ProjectPointToNavigation(SpawnLocation, ProjectedLocation, FVector(200.f, 200.f, 200.f)))
			{
				SpawnLocation = ProjectedLocation.Location;
				SpawnLocation.Z += 50.f; // 지면 위 스폰 유도
			}
		}

		ACharacter* EliteMinion = World->SpawnActor<ACharacter>(
			Settings.EliteMinionSpawnData.MinionClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (EliteMinion)
		{
			EliteMinion->SpawnDefaultController();

			BO_LOG_AI(Log, "Successfully spawned Elite Minion at %s", *SpawnLocation.ToString())
		}
		else
		{
			BO_LOG_AI(Error, "Failed to spawn Elite Minion of class %s at %s!", *Settings.EliteMinionSpawnData.MinionClass->GetName(), *SpawnLocation.ToString());
		}
	}
}
