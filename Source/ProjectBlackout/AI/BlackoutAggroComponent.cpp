// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BlackoutAggroComponent.h"

#include "BlackoutGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "BlackoutBaseAttributeSet.h"
#include "Characters/BlackoutPlayerCharacter.h"


// Sets default values for this component's properties
UBlackoutAggroComponent::UBlackoutAggroComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.

	// 틱 사용안함 - 재평가는 타이머
	PrimaryComponentTick.bCanEverTick = false;
}


void UBlackoutAggroComponent::RecordDamage(APawn* Source, float Amount)
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !IsValid(Source) || Amount <= 0.0f)
	{
		return;
	}
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	DamageRecords.FindOrAdd(Source).Add({Now, Amount});
}

void UBlackoutAggroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReEvalTimerHandle);
	}

	if (OwnerASC && TargetChangeTagHandle.IsValid())
	{
		OwnerASC->RegisterGameplayTagEvent(TargetChangeTag,
		                                   EGameplayTagEventType::NewOrRemoved).
		          Remove(TargetChangeTagHandle);
	}
	OwnerASC = nullptr;
	CurrentTarget = nullptr;
	DamageRecords.Empty();

	Super::EndPlay(EndPlayReason);
}

// Called when the game starts
void UBlackoutAggroComponent::BeginPlay()
{
	Super::BeginPlay();

	// AI 결정은 서버
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	// 소유 Pawn 의 ASC 캐시 
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		OwnerASC = ASI->GetAbilitySystemComponent();
	}

	if (OwnerASC)
	{
		TargetChangeTag = BlackoutGameplayTags::Ability_TargetChange;
		TargetChangeTagHandle = OwnerASC->RegisterGameplayTagEvent(
			TargetChangeTag, EGameplayTagEventType::NewOrRemoved).AddUObject(
			this, &UBlackoutAggroComponent::OnTargetChangeTagChanged);
	}

	// 주기 평가 타이머 시작
	GetWorld()->GetTimerManager().SetTimer(ReEvalTimerHandle, this,
	                                       &UBlackoutAggroComponent::ReEvaluate,
	                                       ReEvalInterval, true);
}

void UBlackoutAggroComponent::ReEvaluate()
{
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// 오래된 데미지 기록 / 죽은 키 정리
	for (auto It = DamageRecords.CreateIterator(); It; ++It)
	{
		It->Value.RemoveAll([&](const FDamageRecord& R)
		{
			return Now - R.Timestamp > DPSWindowDuration;
		});
		if (!It->Key.IsValid() || It->Value.Num() == 0)
		{
			It.RemoveCurrent();
		}
	}

	float BestScore = -1.0f;
	APawn* Best = CalculateBestTarget(BestScore);

	// 현재 타겟이 그대로 이면 유지
	if (Best == CurrentTarget)
	{
		return;
	}

	// 현재 타겟이 무효면 교체
	if (IsValidTarget(CurrentTarget))
	{
		const float CurrentScore = CalculateScore(CurrentTarget);
		const bool bBeatsMargin = BestScore > CurrentScore * (1.0f +
			SwitchScoreMargin);
		const bool bDwellPassed = (Now - LastSwitchTime) > MinDwellTime;

		if (!(bBeatsMargin && bDwellPassed))
		{
			return; // 교체 보류
		}
	}

	// 교체
	CurrentTarget = Best;
	LastSwitchTime = Now;
	OnTargetChanged.Broadcast(CurrentTarget);
}

APawn* UBlackoutAggroComponent::CalculateBestTarget(float& OutBestScore) const
{
	OutBestScore = -1.0f;
	APawn* Best = nullptr;

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = World->
		     GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}

		APawn* Candidate = PC->GetPawn();
		if (!IsValidTarget(Candidate))
		{
			continue;
		}
		const float Score = CalculateScore(Candidate);
		if (Score > OutBestScore)
		{
			OutBestScore = Score;
			Best = Candidate;
		}
	}

	return Best;
}

float UBlackoutAggroComponent::CalculateScore(APawn* Target) const
{
	APawn* Owner = Cast<APawn>(GetOwner());
	if (!IsValid(Target) || !Owner)
	{
		return 0.0f;
	}
	float Score = 0.0f;

	// 거리 가까울수록 높음
	const float Dist = FVector::Dist(Owner->GetActorLocation(),
	                                 Target->GetActorLocation());
	const float DistScore = FMath::Clamp(1.0f - (Dist / MaxAggroRange), 0.0f,
	                                     1.0f);
	Score += DistScore * DistanceWeight;

	// DPS - 누적 대미지
	if (const TArray<FDamageRecord>* Records = DamageRecords.Find(Target))
	{
		const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		float WindowDamage = 0.0f;
		for (const FDamageRecord& R : *Records)
		{
			if (Now - R.Timestamp <= DPSWindowDuration)
			{
				WindowDamage += R.Amount;
			}
		}
		const float DPS = WindowDamage / DPSWindowDuration;
		Score += FMath::Min(DPS / 100.0f, 1.0f) * DPSWeight;
	}

	// 저체력
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Target))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			const float HP = ASC->GetNumericAttribute(
				UBlackoutBaseAttributeSet::GetHealthAttribute());
			const float MaxHP = ASC->GetNumericAttribute(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute());

			if (MaxHP > 0.0f)
			{
				Score += (1.0f - HP / MaxHP) * LowHPWeight;
			}
		}
	}

	return Score;
}

bool UBlackoutAggroComponent::IsValidTarget(APawn* Target) const
{
	// 플레이어 캐릭터만
	ABlackoutPlayerCharacter* Player = Cast<ABlackoutPlayerCharacter>(Target);
	if (!IsValid(Player))
	{
		return false;
	}

	APawn* Owner = Cast<APawn>(GetOwner());
	if (!Owner)
	{
		return false;
	}

	// 사거리 
	const float DistSq = FVector::DistSquared(Owner->GetActorLocation(),
	                                          Player->GetActorLocation());
	if (DistSq > MaxAggroRange * MaxAggroRange)
	{
		return false;
	}

	// 생존 ( HP >0 )
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Player))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			if (ASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Downed)
				|| ASC->
				HasMatchingGameplayTag(BlackoutGameplayTags::State_Dead))
			{
				return false;
			}
		}
	}
	return true;
}

void UBlackoutAggroComponent::OnTargetChangeTagChanged(const FGameplayTag Tag,
	int32 NewCount)
{
	// 회피 종료 시 재평가
	if (NewCount == 0)
	{
		ReEvaluate();
	}
}
