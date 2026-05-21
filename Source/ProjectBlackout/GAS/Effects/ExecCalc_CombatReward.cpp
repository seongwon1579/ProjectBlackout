#include "GAS/Effects/ExecCalc_CombatReward.h"
#include "GAS/Attributes/BlackoutBaseAttributeSet.h"
#include "Combat/BlackoutCombatRewardSettings.h"
#include "GameplayTags/BlackoutGameplayTags.h"
#include "Framework/BlackoutPlayerState.h"
#include "Items/BlackoutDropItem.h"
#include "Pool/BlackoutPoolSubsystem.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Characters/BlackoutBossCharacter.h"
#include "Characters/BlackoutEnemyCharacter.h"
#include "Characters/BlackoutPlayerCharacter.h"
#include "Core/BlackoutLog.h"
#include "Data/BOCharacterData.h"
#include "Data/BORewardTableData.h"
#include "GameFramework/Pawn.h"
#include "GameplayEffect.h"
#include "Engine/World.h"

namespace
{
	FVector ResolveGroundedDropLocation(
		UWorld* World,
		AActor* TargetAvatar,
		const FVector& CandidateLocation,
		float TraceUpDistance,
		float TraceDownDistance,
		float GroundOffset)
	{
		if (!World)
		{
			return CandidateLocation + FVector(0.0f, 0.0f, GroundOffset);
		}

		const FVector TraceStart = CandidateLocation + FVector(0.0f, 0.0f, FMath::Max(TraceUpDistance, 0.0f));
		const FVector TraceEnd = CandidateLocation - FVector(0.0f, 0.0f, FMath::Max(TraceDownDistance, 0.0f));

		FHitResult GroundHit;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CombatRewardDropGroundTrace), false, TargetAvatar);
		QueryParams.AddIgnoredActor(TargetAvatar);

		if (World->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams)
			&& GroundHit.bBlockingHit)
		{
			return GroundHit.ImpactPoint + FVector(0.0f, 0.0f, GroundOffset);
		}

		BO_LOG_CORE(Verbose, TEXT("UExecCalc_CombatReward: 드롭 바닥 탐색 실패. Target=%s Candidate=%s"),
			*GetNameSafe(TargetAvatar),
			*CandidateLocation.ToString());
		return CandidateLocation + FVector(0.0f, 0.0f, GroundOffset);
	}
}

UExecCalc_CombatReward::UExecCalc_CombatReward()
{
	// 기본 드롭 아이템 블루프린트 클래스는 에디터에서 오버라이드할 수 있도록 nullptr로 두되,
	// Execute 시점에 폴백 처리를 보장합니다.
	DropItemClass = nullptr;
}

void UExecCalc_CombatReward::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	(void)OutExecutionOutput;
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	TrySpawnRewardDropInternal(
		ExecutionParams.GetOwningSpec(),
		SourceASC,
		TargetASC,
		DropItemClass,
		DropScatterRadius,
		DropGroundTraceUpDistance,
		DropGroundTraceDownDistance,
		DropGroundOffset,
		RewardTable);
}

bool UExecCalc_CombatReward::ApplyConfiguredRewardEffect(
	const FGameplayEffectSpecHandle& DamageSpecHandle,
	UAbilitySystemComponent* TargetASC)
{
	if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid() || !TargetASC)
	{
		BO_LOG_CORE(Warning, TEXT("UExecCalc_CombatReward: 보상 GE 적용 실패. DamageSpec 또는 TargetASC가 유효하지 않습니다. SpecValid=%d, DataValid=%d, TargetASC=%s"),
			DamageSpecHandle.IsValid() ? 1 : 0,
			DamageSpecHandle.Data.IsValid() ? 1 : 0,
			*GetNameSafe(TargetASC));
		return false;
	}

	const FGameplayEffectContextHandle& ContextHandle = DamageSpecHandle.Data->GetContext();
	UAbilitySystemComponent* SourceASC = ContextHandle.GetInstigatorAbilitySystemComponent();
	if (!SourceASC)
	{
		if (AActor* InstigatorActor = ContextHandle.GetOriginalInstigator())
		{
			SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstigatorActor);
		}
	}
	if (!SourceASC)
	{
		if (AActor* InstigatorActor = ContextHandle.GetInstigator())
		{
			SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(InstigatorActor);
		}
	}
	if (!SourceASC)
	{
		if (AActor* EffectCauser = ContextHandle.GetEffectCauser())
		{
			SourceASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(EffectCauser);
		}
	}

	if (!SourceASC)
	{
		BO_LOG_CORE(Warning, TEXT("UExecCalc_CombatReward: 보상 GE 적용 실패. SourceASC를 찾지 못했습니다. Instigator=%s, OriginalInstigator=%s, EffectCauser=%s, TargetASC=%s, Tags=%s"),
			*GetNameSafe(ContextHandle.GetInstigator()),
			*GetNameSafe(ContextHandle.GetOriginalInstigator()),
			*GetNameSafe(ContextHandle.GetEffectCauser()),
			*GetNameSafe(TargetASC),
			*DamageSpecHandle.Data->GetDynamicAssetTags().ToStringSimple());
		return false;
	}

	const UBlackoutCombatRewardSettings* RewardSettings = UBlackoutCombatRewardSettings::Get();
	TSubclassOf<UGameplayEffect> RewardEffectClass = RewardSettings
		? RewardSettings->CombatRewardEffectClass.LoadSynchronous()
		: nullptr;

	if (!RewardEffectClass)
	{
		BO_LOG_CORE(Warning, TEXT("UExecCalc_CombatReward: CombatRewardEffectClass가 설정되지 않아 보상 판정을 건너뜁니다."));
		return false;
	}

	FGameplayEffectSpecHandle RewardSpecHandle = SourceASC->MakeOutgoingSpec(
		RewardEffectClass,
		DamageSpecHandle.Data->GetLevel(),
		ContextHandle);
	if (!RewardSpecHandle.IsValid() || !RewardSpecHandle.Data.IsValid())
	{
		BO_LOG_CORE(Error, TEXT("UExecCalc_CombatReward: 보상 GameplayEffect Spec 생성에 실패했습니다. Effect=%s"),
			*GetNameSafe(RewardEffectClass.Get()));
		return false;
	}

	// 마지막 타격 Spec의 조건 태그를 보상 GE로 복사해 ExecCalc가 같은 판정 기준을 사용하게 합니다.
	TArray<FGameplayTag> DynamicTags;
	DamageSpecHandle.Data->GetDynamicAssetTags().GetGameplayTagArray(DynamicTags);
	for (const FGameplayTag& DynamicTag : DynamicTags)
	{
		RewardSpecHandle.Data->AddDynamicAssetTag(DynamicTag);
	}

	SourceASC->ApplyGameplayEffectSpecToTarget(*RewardSpecHandle.Data.Get(), TargetASC);
	return true;
}

bool UExecCalc_CombatReward::TrySpawnRewardDropInternal(
	const FGameplayEffectSpec& RewardSpec,
	UAbilitySystemComponent* SourceASC,
	UAbilitySystemComponent* TargetASC,
	TSubclassOf<ABlackoutDropItem> InDropItemClass,
	float InDropScatterRadius,
	float InDropGroundTraceUpDistance,
	float InDropGroundTraceDownDistance,
	float InDropGroundOffset,
	UBORewardTableData* InRewardTable)
{
	if (!SourceASC || !TargetASC)
	{
		return false;
	}

	// 1. 타겟(피해자)이 정말 사망했거나 사망 진행 중인지 확인 (체력이 0 이하이거나 State.Dead 태그 보유 시)
	const float CurrentHealth = TargetASC->GetNumericAttribute(UBlackoutBaseAttributeSet::GetHealthAttribute());
	const bool bIsDead = CurrentHealth <= 0.0f || TargetASC->HasMatchingGameplayTag(BlackoutGameplayTags::State_Dead);

	if (!bIsDead)
	{
		BO_LOG_CORE(Verbose, TEXT("UExecCalc_CombatReward: 타겟이 사망 상태가 아니어서 보상 드롭을 건너뜁니다. TargetASC=%s, Health=%.2f"),
			*GetNameSafe(TargetASC),
			CurrentHealth);
		return false;
	}

	// 2. 가해자(Source)의 PlayerState를 통해 적합한 플레이어인지 판정
	ABlackoutPlayerState* SourcePS = nullptr;
	if (const AActor* SourceAvatar = SourceASC->GetAvatarActor())
	{
		if (const APawn* SourcePawn = Cast<APawn>(SourceAvatar))
		{
			SourcePS = SourcePawn->GetPlayerState<ABlackoutPlayerState>();
		}
		if (!SourcePS)
		{
			SourcePS = Cast<ABlackoutPlayerState>(SourceASC->GetOwner());
		}
	}

	// 플레이어가 몬스터를 처치한 경우가 아니면 보상을 지급하지 않음
	if (!SourcePS)
	{
		BO_LOG_CORE(Verbose, TEXT("UExecCalc_CombatReward: Source PlayerState를 찾지 못해 보상 판정을 건너뜁니다. SourceASC=%s"),
			*GetNameSafe(SourceASC));
		return false;
	}

	// 3. GameplayEffect Spec 내부의 킬 조건 태그 검증
	// 다각도로 킬 태그를 검색하는 헬퍼 람다 (DynamicAssetTags 및 캡처된 소스/타겟 태그 컨테이너 스캔)
	auto HasKillTag = [&RewardSpec](const FGameplayTag& KillTag) -> bool
	{
		if (RewardSpec.GetDynamicAssetTags().HasTagExact(KillTag))
		{
			return true;
		}

		const FGameplayTagContainer* SourceTags = RewardSpec.CapturedSourceTags.GetAggregatedTags();
		if (SourceTags && SourceTags->HasTagExact(KillTag))
		{
			return true;
		}

		const FGameplayTagContainer* TargetTags = RewardSpec.CapturedTargetTags.GetAggregatedTags();
		if (TargetTags && TargetTags->HasTagExact(KillTag))
		{
			return true;
		}

		return false;
	};

	// 4. 가해자의 클래스(병과) 태그와 킬 조건 태그 매칭 평가
	bool bConditionMet = false;
	FGameplayTag ClassTag = SourcePS->SelectedClassTag;
	if (!ClassTag.IsValid())
	{
		if (const ABlackoutPlayerCharacter* SourceCharacter = Cast<ABlackoutPlayerCharacter>(SourceASC->GetAvatarActor()))
		{
			if (const UBOCharacterData* SourceCharacterData = SourceCharacter->GetCharacterData())
			{
				ClassTag = SourceCharacterData->ClassTag;
			}
		}
	}

	if (ClassTag == BlackoutGameplayTags::Character_Class_Assault)
	{
		// 탱커/어썰트: 근접 공격 처치 (Kill_Melee)
		bConditionMet = HasKillTag(BlackoutGameplayTags::Kill_Melee);
	}
	else if (ClassTag == BlackoutGameplayTags::Character_Class_Demolition)
	{
		// 광역딜/데몰리션: 3마리 이상 다중 처치 (Kill_MultiTarget_Count3)
		bConditionMet = HasKillTag(BlackoutGameplayTags::Kill_MultiTarget_Count3);
	}
	else if (ClassTag == BlackoutGameplayTags::Character_Class_Sniper)
	{
		// 저격/스나이퍼: 약점/헤드샷 치명타 처치 (Kill_WeakSpot)
		bConditionMet = HasKillTag(BlackoutGameplayTags::Kill_WeakSpot);
	}

	// 킬 조건을 충족하지 못하면 보상을 스폰하지 않음
	if (!bConditionMet)
	{
		BO_LOG_CORE(Verbose, TEXT("UExecCalc_CombatReward: 보상 조건 불일치. Player=%s, Class=%s, Tags=%s"),
			*SourcePS->GetPlayerName(),
			*ClassTag.ToString(),
			*RewardSpec.GetDynamicAssetTags().ToStringSimple());
		return false;
	}

	// 5. 서버 권한 하에 풀링 서브시스템을 통한 드롭 아이템 상자 스폰
	AActor* TargetAvatar = TargetASC->GetAvatarActor();
	if (!TargetAvatar)
	{
		return false;
	}

	// 일반 적 처치 보상만 월드 드롭으로 전환합니다. 플레이어/보스 사망은 별도 흐름에서 처리합니다.
	if (!Cast<ABlackoutEnemyCharacter>(TargetAvatar) || Cast<ABlackoutBossCharacter>(TargetAvatar))
	{
		BO_LOG_CORE(Verbose, TEXT("UExecCalc_CombatReward: 일반 적이 아니거나 보스라서 보상 드롭을 건너뜁니다. Target=%s"),
			*GetNameSafe(TargetAvatar));
		return false;
	}

	UWorld* World = TargetAvatar->GetWorld();
	if (!World)
	{
		return false;
	}

	// 데디서버 환경에서만 아이템 상자 스폰을 직접 수행
	if (!World->IsNetMode(NM_Client))
	{
		UBlackoutPoolSubsystem* PoolSubsystem = World->GetSubsystem<UBlackoutPoolSubsystem>();
		if (!PoolSubsystem)
		{
			BO_LOG_CORE(Error, TEXT("UExecCalc_CombatReward: 풀링 서브시스템을 획득하지 못했습니다."));
			return false;
		}

		// 보상 GE에 설정된 BP ExecCalc의 DropItemClass를 사용하고, 비어 있다면 C++ 원본 베이스 클래스로 안전하게 폴백
		TSubclassOf<ABlackoutDropItem> SpawnClass = InDropItemClass
			? InDropItemClass
			: TSubclassOf<ABlackoutDropItem>(ABlackoutDropItem::StaticClass());

		// 6. 드롭할 아이템 종류 및 보상 수치 결정 (데이터 에셋 기반 가중치 연산)
		EBlackoutDropItemType SelectedType = EBlackoutDropItemType::PrimaryAmmo;
		float SelectedSupplyRatio = 0.2f;

		if (InRewardTable && InRewardTable->RewardConfigs.Num() > 0)
		{
			// 가중치 합 계산
			float TotalWeight = 0.0f;
			for (const FBlackoutRewardItemConfig& Config : InRewardTable->RewardConfigs)
			{
				TotalWeight += FMath::Max(Config.DropWeight, 0.0f);
			}

			if (TotalWeight > 0.0f)
			{
				const float RandVal = FMath::FRandRange(0.0f, TotalWeight);
				float CurrentWeightSum = 0.0f;
				bool bSelected = false;

				for (const FBlackoutRewardItemConfig& Config : InRewardTable->RewardConfigs)
				{
					CurrentWeightSum += FMath::Max(Config.DropWeight, 0.0f);
					if (RandVal <= CurrentWeightSum)
					{
						SelectedType = Config.ItemType;
						SelectedSupplyRatio = Config.SupplyRatio;
						bSelected = true;
						break;
					}
				}

				// 혹시 연산 오차 등으로 선택되지 않았을 경우 마지막 구성 요소로 폴백
				if (!bSelected)
				{
					const FBlackoutRewardItemConfig& LastConfig = InRewardTable->RewardConfigs.Last();
					SelectedType = LastConfig.ItemType;
					SelectedSupplyRatio = LastConfig.SupplyRatio;
				}
			}
			else
			{
				// 가중치 합이 0일 경우 무작위 선택
				const int32 RandIndex = FMath::RandRange(0, InRewardTable->RewardConfigs.Num() - 1);
				const FBlackoutRewardItemConfig& RandomConfig = InRewardTable->RewardConfigs[RandIndex];
				SelectedType = RandomConfig.ItemType;
				SelectedSupplyRatio = RandomConfig.SupplyRatio;
			}
		}
		else
		{
			// 데이터 에셋이 없거나 비어 있을 경우 기존 하드코딩 확률(40% / 40% / 20%) 및 충전비율(0.2f)로 폴백
			const float RandVal = FMath::FRand();
			if (RandVal < 0.4f)
			{
				SelectedType = EBlackoutDropItemType::PrimaryAmmo;
				SelectedSupplyRatio = 0.2f;
			}
			else if (RandVal < 0.8f)
			{
				SelectedType = EBlackoutDropItemType::SecondaryAmmo;
				SelectedSupplyRatio = 0.2f;
			}
			else
			{
				SelectedType = EBlackoutDropItemType::Consumable;
				SelectedSupplyRatio = 0.0f; // 소모품은 비율 미사용
			}

			BO_LOG_CORE(Warning, TEXT("UExecCalc_CombatReward: RewardTable이 비어있어 기본 하드코딩 드롭 테이블로 폴백합니다."));
		}

		// 사망한 몬스터 위치 주변에서 후보점을 정한 뒤, 아래 방향 트레이스로 실제 바닥 위치에 맞춰 생성합니다.
		const float ScatterRadius = FMath::Max(InDropScatterRadius, 0.0f);
		const FVector ScatterOffset(FMath::RandRange(-ScatterRadius, ScatterRadius), FMath::RandRange(-ScatterRadius, ScatterRadius), 0.0f);
		const FVector CandidateLocation = TargetAvatar->GetActorLocation() + ScatterOffset;
		const FVector SpawnLocation = ResolveGroundedDropLocation(
			World,
			TargetAvatar,
			CandidateLocation,
			InDropGroundTraceUpDistance,
			InDropGroundTraceDownDistance,
			InDropGroundOffset);
		const FRotator SpawnRotation(0.0f, TargetAvatar->GetActorRotation().Yaw, 0.0f);
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

		AActor* SpawnedActor = PoolSubsystem->SpawnFromPool(SpawnClass, SpawnTransform);
		ABlackoutDropItem* DropItem = Cast<ABlackoutDropItem>(SpawnedActor);
		if (DropItem)
		{
			// 드롭 상태 타입 및 획득 보상 비율 동적 주입
			DropItem->InitializeDropReward(SelectedType, SelectedSupplyRatio);
			DropItem->SnapToGround(TargetAvatar);

			BO_LOG_CORE(Log, TEXT("처치 보상 드롭 성공: Player=%s, Class=%s, DroppedType=%d, SupplyRatio=%.2f, Location=%s"),
				*SourcePS->GetPlayerName(),
				*ClassTag.ToString(),
				(int32)SelectedType,
				SelectedSupplyRatio,
				*DropItem->GetActorLocation().ToString());
			return true;
		}
		else
		{
			BO_LOG_CORE(Error, TEXT("UExecCalc_CombatReward: 스폰된 액터가 ABlackoutDropItem 계열이 아닙니다. Spawned=%s"),
				*GetNameSafe(SpawnedActor));
		}
	}

	return false;
}
