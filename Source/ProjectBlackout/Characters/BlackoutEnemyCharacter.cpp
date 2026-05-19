#include "BlackoutEnemyCharacter.h"

#include "AudioMixerBlueprintLibrary.h"
#include "BlackoutAbilitySystemComponent.h"
#include "Attributes/BlackoutBaseAttributeSet.h"
#include "Data/BOMinionData.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "BlackoutLog.h"
#include "UI/BlackoutMinionHealthBarWidget.h"

ABlackoutEnemyCharacter::ABlackoutEnemyCharacter()
{
	// 적은 자기 자신이 ASC를 소유
	AbilitySystemComponent = CreateDefaultSubobject<
		UBlackoutAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(
		EGameplayEffectReplicationMode::Minimal);

	BaseAttributeSet = CreateDefaultSubobject<UBlackoutBaseAttributeSet>(
		TEXT("BaseAttributeSet"));
	AbilitySystemComponent->AddAttributeSetSubobject(BaseAttributeSet.Get());

	MinionHealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("MinionHealthBarComponent"));
	MinionHealthBarComponent->SetupAttachment(GetRootComponent());
	MinionHealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	MinionHealthBarComponent->SetDrawSize(MinionHealthBarDrawSize);
	MinionHealthBarComponent->SetDrawAtDesiredSize(false);
	MinionHealthBarComponent->SetTwoSided(true);
	MinionHealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MinionHealthBarComponent->SetGenerateOverlapEvents(false);
	MinionHealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, MinionHealthBarHeight));
	MinionHealthBarComponent->SetVisibility(false);
}

void ABlackoutEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		// 기본 AttributeSet 추가 및 MinionData 수치 주입
		if (BaseAttributeSet && MinionData)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
				MinionData->MaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetHealthAttribute(),
				MinionData->MaxHealth);
		}

		// 데이터 에셋에 추가된 GA 일괄 부여 (서버 권한 가드는 GiveDefaultAbilities 내부 처리)
		if (MinionData && MinionData->GrantedAbilities.Num() > 0)
		{
			AbilitySystemComponent->GiveDefaultAbilities(
				MinionData->GrantedAbilities);
		}
	}

	InitializeMinionHealthBar();
}

void ABlackoutEnemyCharacter::OnSpawnFromPool_Implementation()
{
	bIsDead = false;
	bIsDowned = false;

	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// HP / GA 둘다 재초기화
	if (AbilitySystemComponent && MinionData)
	{
		if (BaseAttributeSet)
		{
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetMaxHealthAttribute(),
				MinionData->MaxHealth);
			AbilitySystemComponent->SetNumericAttributeBase(
				UBlackoutBaseAttributeSet::GetHealthAttribute(),
				MinionData->MaxHealth);
		}
		if (MinionData->GrantedAbilities.Num() > 0)
		{
			AbilitySystemComponent->GiveDefaultAbilities(
				MinionData->GrantedAbilities);
		}
	}

	InitializeMinionHealthBar();

	BO_LOG_POOL(Verbose, "SpawnFromPool: %s", *GetName());
}

void ABlackoutEnemyCharacter::OnReturnToPool_Implementation()
{
	HideMinionHealthBar();

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// GE·GA 전체 제거 (다음 스폰 시 깨끗한 상태 보장)
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ClearAllAbilitiesAndEffects();
	}

	BO_LOG_POOL(Verbose, "ReturnToPool: %s", *GetName());
}

void ABlackoutEnemyCharacter::InitializeMinionHealthBar()
{
	if (!MinionHealthBarComponent)
	{
		return;
	}

	if (!ShouldUseMinionHealthBar())
	{
		HideMinionHealthBar();
		return;
	}

	ApplyMinionHealthBarLayout();
	if (!MinionHealthBarWidgetClass)
	{
		BO_LOG_CORE(Warning,
			"미니언 체력바 초기화 실패: MinionHealthBarWidgetClass가 지정되지 않았습니다. Actor=%s",
			*GetNameSafe(this));
		MinionHealthBarComponent->SetVisibility(false);
		return;
	}

	MinionHealthBarComponent->SetWidgetClass(MinionHealthBarWidgetClass);
	MinionHealthBarComponent->SetVisibility(true);
	MinionHealthBarComponent->InitWidget();

	UBlackoutMinionHealthBarWidget* HealthBarWidget =
		Cast<UBlackoutMinionHealthBarWidget>(MinionHealthBarComponent->GetUserWidgetObject());
	if (!HealthBarWidget)
	{
		BO_LOG_CORE(Warning,
			"미니언 체력바 초기화 실패: WidgetClass가 비어 있거나 UBlackoutMinionHealthBarWidget 기반이 아닙니다. Actor=%s WidgetClass=%s",
			*GetNameSafe(this),
			*GetNameSafe(MinionHealthBarComponent->GetWidgetClass()));
		MinionHealthBarComponent->SetVisibility(false);
		return;
	}

	HealthBarWidget->BindToAbilitySystem(GetAbilitySystemComponent());
}

void ABlackoutEnemyCharacter::HideMinionHealthBar()
{
	if (!MinionHealthBarComponent)
	{
		return;
	}

	if (UBlackoutMinionHealthBarWidget* HealthBarWidget =
		Cast<UBlackoutMinionHealthBarWidget>(MinionHealthBarComponent->GetUserWidgetObject()))
	{
		HealthBarWidget->SetForceHidden(true);
	}

	MinionHealthBarComponent->SetVisibility(false);
}

void ABlackoutEnemyCharacter::ApplyMinionHealthBarLayout()
{
	if (!MinionHealthBarComponent)
	{
		return;
	}

	MinionHealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, MinionHealthBarHeight));
	MinionHealthBarComponent->SetDrawSize(FVector2D(
		FMath::Max(MinionHealthBarDrawSize.X, 1.0f),
		FMath::Max(MinionHealthBarDrawSize.Y, 1.0f)));
}

void ABlackoutEnemyCharacter::OnDeath()
{
	HideMinionHealthBar();
	Super::OnDeath();
}

bool ABlackoutEnemyCharacter::ShouldUseMinionHealthBar() const
{
	return MinionData != nullptr;
}
