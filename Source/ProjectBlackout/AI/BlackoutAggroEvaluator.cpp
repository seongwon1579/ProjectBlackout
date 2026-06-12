// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutAggroEvaluator.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "BlackoutBaseAttributeSet.h"
#include "BlackoutGameplayTags.h"
#include "GameFramework/GameModeBase.h"

UWorld* UBlackoutAggroEvaluator::GetWorld() const
{
	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

void UBlackoutAggroEvaluator::Initialize(AAIController* InAIController, UAbilitySystemComponent* InASC)
{
	// CachedOwnerAIController = InAIController;
	// CachedASC = InASC;
	//
	// UE_LOG(LogTemp, Warning, TEXT("UBlackoutAggroEvaluator::Initialize"))
	//
	// if (!InAIController || !InAIController->HasAuthority()) return;
	//
	// RegisterTagEvents();
	// RegisterPlayerEvents();
	//
	// UpdateTarget();
	CachedOwnerAIController = InAIController;
	CachedASC = InASC;
    
	UE_LOG(LogTemp, Warning, TEXT("UBlackoutAggroEvaluator::Initialize"))

	if (!InAIController || !InAIController->HasAuthority()) return;

	RegisterTagEvents();

	// [수정] 즉시 바인딩/타겟팅을 시도하지 않고, 플레이어가 월드에 완전히 올라올 때까지 타이머를 돌립니다.
	if (UWorld* World = GetWorld())
	{
		// 0.1초마다 TryInitialTargeting을 호출합니다.
		World->GetTimerManager().SetTimer(StartupTimerHandle, this, &UBlackoutAggroEvaluator::TryInitialTargeting, 0.1f, true);
	}
}

void UBlackoutAggroEvaluator::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StartupTimerHandle);
	}
	
	UnregisterTagEvents();
	UnregisterPlayerEvents();

	CachedASC = nullptr;
	CachedOwnerAIController = nullptr;
	CombatDataMap.Empty();
}

void UBlackoutAggroEvaluator::UpdateTarget()
{
	if (!CachedOwnerAIController) return;
	
	UE_LOG(LogTemp, Warning, TEXT("UpdateTarget"))

	APawn* BestTarget = CalculateBestTarget(nullptr);

	if (BestTarget && BestTarget != CurrentTarget.Get())
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateTarget BestTarget && BestTarget != CurrentTarget.Get()"))
		
		CurrentTarget = BestTarget;
		WatchTargetDownState(BestTarget);
		OnAggroTargetChanged.Broadcast(BestTarget);
	}
}

void UBlackoutAggroEvaluator::OnAggroTargetChangeTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount == 0)
	{
		UpdateTarget();
	}
}

void UBlackoutAggroEvaluator::OnPlayerPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (IsValid(NewPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("플레이어 폰 빙의 감지! 보스전 어그로 시스템 가동. 타겟: %s"), *NewPawn->GetName());
		UpdateTarget();
	}
}

void UBlackoutAggroEvaluator::OnPostLogin(AGameModeBase* GameMode, APlayerController* NewPC)
{
	UE_LOG(LogTemp, Warning, TEXT("OnPostLogin"))
	if (!NewPC) return;

	UE_LOG(LogTemp, Warning, TEXT("OnPostLogin NewPC"))
	NewPC->OnPossessedPawnChanged.AddDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);

	if (NewPC->GetPawn())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPostLogin NewPC->GetPawn()"))
		UpdateTarget();
	}
}

void UBlackoutAggroEvaluator::RecordDamage(APawn* Source, float Amount)
{
	if (!IsValid(Source) || Amount <= 0) return;

	FPlayerCombatData& Data = CombatDataMap.FindOrAdd(Source);
	FDamageRecord NewRecord;

	UWorld* World = GetWorld();
	if (!World) return;

	NewRecord.Timestamp = World->GetTimeSeconds();
	NewRecord.Amount = Amount;
	Data.DamageRecords.Add(NewRecord);
}

APawn* UBlackoutAggroEvaluator::CalculateBestTarget(APawn* ExcludeTarget) const
{
	if (!CachedOwnerAIController) return nullptr;
	
	UE_LOG(LogTemp, Warning, TEXT("CalculateBestTarget"))

	APawn* Owner = CachedOwnerAIController->GetPawn();
	if (!Owner || !GetWorld()) return nullptr;

	APawn* BestTarget = nullptr;

	float BestScore = -1.f;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		UE_LOG(LogTemp, Warning, TEXT("CalculateBestTarget"))
		
		APlayerController* PC = It->Get();
		if (!PC) continue;
		
		UE_LOG(LogTemp, Warning, TEXT("CalculateBestTarget PC"))

		APawn* Target = PC->GetPawn();
		if (!IsValid(Target)) continue;
		
		UE_LOG(LogTemp, Warning, TEXT("CalculateBestTarget Pawn"))
		
		if (IsTargetInvalid(Target)) continue;

		// 최대사거리를 벗어난 Target은 제외
		const float DistSQ = FVector::DistSquared(Target->GetActorLocation(), Owner->GetActorLocation());
		if (DistSQ > MaxAggroRange * MaxAggroRange) continue;

		const float Score = CalculateAggroScore(Target);

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}
	return BestTarget;
}

void UBlackoutAggroEvaluator::RegisterTagEvents()
{
	if (!CachedASC) return;

	TargetChangeTag = BlackoutGameplayTags::Ability_TargetChange;
	TargetChangeTagChangedHandle = CachedASC->RegisterGameplayTagEvent(
		TargetChangeTag,
		EGameplayTagEventType::NewOrRemoved
	).AddUObject(this, &UBlackoutAggroEvaluator::OnAggroTargetChangeTagChanged);
}

void UBlackoutAggroEvaluator::UnregisterTagEvents()
{
	if (!CachedASC) return;

	CachedASC->RegisterGameplayTagEvent(
		TargetChangeTag,
		EGameplayTagEventType::NewOrRemoved
	).Remove(TargetChangeTagChangedHandle);
}

void UBlackoutAggroEvaluator::RegisterPlayerEvents()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("RegisterPlayerEvents"))
	// 	if (APlayerController* PC = It->Get())
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("RegisterPlayerEvents PC"))
	// 		PC->OnPossessedPawnChanged.AddDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);
	// 	}
	// }
	//
	// UE_LOG(LogTemp, Warning, TEXT("RegisterPlayerEvents Next"))
	// PostLoginHandle = FGameModeEvents::GameModePostLoginEvent.AddUObject(
	// 	this, &UBlackoutAggroEvaluator::OnPostLogin);
	
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->OnPossessedPawnChanged.RemoveDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);
			PC->OnPossessedPawnChanged.AddDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);
		}
	}
}

void UBlackoutAggroEvaluator::UnregisterPlayerEvents()
{
	FGameModeEvents::GameModePostLoginEvent.Remove(PostLoginHandle);

	UWorld* World = GetWorld();
	if (!World) return;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			PC->OnPossessedPawnChanged.RemoveDynamic(this, &UBlackoutAggroEvaluator::OnPlayerPawnChanged);
		}
	}
}

void UBlackoutAggroEvaluator::TryInitialTargeting()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UE_LOG(LogTemp, Warning, TEXT("TryInitialTargeting - 플레이어 로딩 대기 중..."))

	RegisterPlayerEvents();

	UpdateTarget();

	if (CurrentTarget.IsValid())
	{
		World->GetTimerManager().ClearTimer(StartupTimerHandle);
		UE_LOG(LogTemp, Log, TEXT("보스전 최초 타겟 확보 성공 및 스타트업 타이머 종료! 타겟: %s"), *CurrentTarget.Get()->GetName());
	}
}

void UBlackoutAggroEvaluator::WatchTargetDownState(APawn* Target)
{
	UnWatchTargetDownState();

	if (!IsValid(Target)) return;

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	if (!ASI) return;

	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return;

	DownTag = BlackoutGameplayTags::State_Downed;
	CurrentTargetASC = ASC;

	TargetDownTagHandle = ASC->RegisterGameplayTagEvent(DownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
		this, &UBlackoutAggroEvaluator::OnTargetDownTagChanged);
	
	if (ASC->HasMatchingGameplayTag(DownTag))
	{
		OnTargetDownTagChanged(DownTag, 1);
	}
}

void UBlackoutAggroEvaluator::UnWatchTargetDownState()
{
	if (CurrentTargetASC.IsValid())
	{
		CurrentTargetASC->RegisterGameplayTagEvent(DownTag, EGameplayTagEventType::NewOrRemoved).Remove(TargetDownTagHandle);
	}
	CurrentTargetASC = nullptr;
}

void UBlackoutAggroEvaluator::OnTargetDownTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		UpdateTarget();
	}
}

bool UBlackoutAggroEvaluator::IsTargetInvalid(APawn* Target) const
{
	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target);
	if (!ASI) return false;
	
	UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
	if (!ASC) return false;
	
	return ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed)
		|| ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Dead);
}

float UBlackoutAggroEvaluator::CalculateAggroScore(APawn* Target) const
{
	if (!IsValid(Target) || !CachedOwnerAIController) return 0.f;

	APawn* Owner = CachedOwnerAIController->GetPawn();
	if (!Owner || !GetWorld()) return 0.f;

	float Score = 0.f;
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// 거리
	const float Dist = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
	const float DistScore = FMath::Clamp(1.f - (Dist / MaxAggroRange), 0.f, 1.f);
	Score += DistScore * DistanceWeight;

	// DPS
	if (FPlayerCombatData* Data = CombatDataMap.Find(Target))
	{
		const float DamageInWindow = Data->GetDamageInWindow(CurrentTime, DPSWindowDuration);
		const float DPS = DamageInWindow / DPSWindowDuration;
		const float DPSScore = FMath::Min(DPS / 100.f, 1.f);
		// TODO: 커브로 변경 필요
		// if (DPSAggroCurve)
		// {
		// 	DPSScore =
		// 		DPSAggroCurve->GetFloatValue(DPS);
		// }
		Score += DPSScore * DPSWeight;
	}

	// 체력
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const float HP = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
			const float MaxHP = ASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetMaxHealthAttribute());

			if (MaxHP > 0.f)
			{
				Score += (1.f - HP / MaxHP) * LowHPWeight;
			}
		}
	}
	return Score;
}
