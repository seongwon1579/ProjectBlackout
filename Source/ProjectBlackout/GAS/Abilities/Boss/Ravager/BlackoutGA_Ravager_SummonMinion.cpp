#include "GAS/Abilities/Boss/Ravager/BlackoutGA_Ravager_SummonMinion.h"

#include "BlackoutBossCharacter.h"
#include "BlackoutGameplayTags.h"
#include "BOEnemySpawnerProjectile.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Kismet/GameplayStatics.h"
#include "AI/BlackoutBossAIController.h"
#include "AbilitySystemComponent.h"
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
	
	UE_LOG(LogTemp, Log, TEXT("[SummonMinion] SetSpawnerProjectiles called. Spawning Spawner Projectiles. Count: %d"), Count);

	for (int32 i = 0; i < Count; i++)
	{
		ThrowSingleSpawnerProjectile(SpawnLocation, BaseRotation, i , Count);
	}

	// 보스 페이즈 판정 및 엘리트 미니언 즉시 스폰 (Phase 2 이상일 때)
	if (ABlackoutBossAIController* BossAIC = Cast<ABlackoutBossAIController>(CachedOwner->GetController()))
	{
		const EBOBossPhase CurrentPhase = BossAIC->GetCurrentPhase();
		UE_LOG(LogTemp, Log, TEXT("[SummonMinion] Current Boss Phase: %d"), (int32)CurrentPhase);

		if (CurrentPhase >= EBOBossPhase::Phase2)
		{
			UE_LOG(LogTemp, Log, TEXT("[SummonMinion] Boss Phase is Phase2 or higher. Immediately spawning elite minions."));
			SpawnEliteMinionsDirectly();
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[SummonMinion] Boss Phase is lower than Phase2. Skipping elite minion spawn."));
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

void UBlackoutGA_Ravager_SummonMinion::SpawnEliteMinionsDirectly()
{
	UE_LOG(LogTemp, Log, TEXT("[SummonMinion] SpawnEliteMinionsDirectly called."));

	if (!CachedOwner || !CachedPatternData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] CachedOwner or CachedPatternData is null inside SpawnEliteMinionsDirectly!"));
		return;
	}

	const FBossMinionSpawnSettings& Settings = CachedPatternData->MinionSettings;
	if (!Settings.EliteMinionSpawnData.MinionClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] EliteMinionSpawnData.MinionClass is null. Please allocate it in the BORavagerPatternData asset."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] World is null inside SpawnEliteMinionsDirectly."));
		return;
	}

	const int32 EliteCount = Settings.EliteSpawnCount;
	const float SpawnRadius = Settings.EliteSpawnRadius;
	const FVector BossLocation = CachedOwner->GetActorLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = CachedOwner;
	SpawnParams.Instigator = CachedOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UE_LOG(LogTemp, Log, TEXT("[SummonMinion] Spawning %d Elite Minions of class %s within radius %.2f around boss location %s"), 
		EliteCount, *Settings.EliteMinionSpawnData.MinionClass->GetName(), SpawnRadius, *BossLocation.ToString());

	for (int32 i = 0; i < EliteCount; i++)
	{
		// 보스 반경 내 무작위 각도 및 거리 오프셋 계산
		const float RandomAngle = FMath::FRand() * 360.f;
		const float RandomRadius = FMath::FRand() * SpawnRadius;

		FVector SpawnLocation = BossLocation + FRotator(0.f, RandomAngle, 0.f).Vector() * RandomRadius;
		SpawnLocation.Z = BossLocation.Z + 50.f; // 지면에 배치하기 위한 오프셋 적용

		ACharacter* EliteMinion = World->SpawnActor<ACharacter>(
			Settings.EliteMinionSpawnData.MinionClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (EliteMinion)
		{
			UE_LOG(LogTemp, Log, TEXT("[SummonMinion] Successfully spawned Elite Minion: %s at %s"), *EliteMinion->GetName(), *SpawnLocation.ToString());
			EliteMinion->SpawnDefaultController();

			// 텔레포트 등장 연출용 GameplayCue 발동
			if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(EliteMinion))
			{
				if (UAbilitySystemComponent* MinionASC = ASI->GetAbilitySystemComponent())
				{
					FGameplayCueParameters CueParams;
					CueParams.Location = SpawnLocation;
					MinionASC->ExecuteGameplayCue(BlackoutGameplayTags::GameplayCue_Wraith_Teleport_End, CueParams);
					UE_LOG(LogTemp, Log, TEXT("[SummonMinion] Executed Wraith_Teleport_End GameplayCue for Elite: %s"), *EliteMinion->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] Minion ASC is null. GameplayCue skipped."));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[SummonMinion] Minion does not implement IAbilitySystemInterface."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[SummonMinion] Failed to spawn Elite Minion of class %s at %s!"), *Settings.EliteMinionSpawnData.MinionClass->GetName(), *SpawnLocation.ToString());
		}
	}
}
