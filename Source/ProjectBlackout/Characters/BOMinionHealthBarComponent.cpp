#include "BOMinionHealthBarComponent.h"

#include "BlackoutLog.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "UI/BlackoutMinionHealthBarWidget.h"

UBOMinionHealthBarComponent::UBOMinionHealthBarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("WidgetComponent"));
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetDrawSize(DrawSize);
	WidgetComponent->SetDrawAtDesiredSize(false);
	WidgetComponent->SetTwoSided(true);
	WidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WidgetComponent->SetGenerateOverlapEvents(false);
	WidgetComponent->SetVisibility(false);
}

void UBOMinionHealthBarComponent::OnRegister()
{
	Super::OnRegister();

	// UActorComponent 는 SceneComponent 가 아니라 직접 부착 불가 — 내부 UWidgetComponent 를
	// owner 의 RootComponent 에 동적 부착한다.
	if (!WidgetComponent)
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	USceneComponent* Root = OwnerActor->GetRootComponent();
	if (!Root)
	{
		return;
	}

	if (!WidgetComponent->IsRegistered())
	{
		WidgetComponent->RegisterComponent();
	}

	if (WidgetComponent->GetAttachParent() != Root)
	{
		WidgetComponent->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
	}

	ApplyLayout();
}

void UBOMinionHealthBarComponent::InitializeFromASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!WidgetComponent)
	{
		return;
	}

	ApplyLayout();

	if (!WidgetClass)
	{
		BO_LOG_CORE(Warning,
			"미니언 체력바 초기화 실패: WidgetClass 가 지정되지 않았습니다. Owner=%s",
			*GetNameSafe(GetOwner()));
		WidgetComponent->SetVisibility(false);
		return;
	}

	WidgetComponent->SetWidgetClass(WidgetClass);
	WidgetComponent->SetVisibility(true);
	WidgetComponent->InitWidget();

	UBlackoutMinionHealthBarWidget* HealthBarWidget =
		Cast<UBlackoutMinionHealthBarWidget>(WidgetComponent->GetUserWidgetObject());
	if (!HealthBarWidget)
	{
		BO_LOG_CORE(Warning,
			"미니언 체력바 초기화 실패: WidgetClass 가 UBlackoutMinionHealthBarWidget 기반이 아닙니다. Owner=%s WidgetClass=%s",
			*GetNameSafe(GetOwner()),
			*GetNameSafe(WidgetComponent->GetWidgetClass()));
		WidgetComponent->SetVisibility(false);
		return;
	}

	HealthBarWidget->BindToAbilitySystem(AbilitySystemComponent);
}

void UBOMinionHealthBarComponent::Hide()
{
	if (!WidgetComponent)
	{
		return;
	}

	if (UBlackoutMinionHealthBarWidget* HealthBarWidget =
		Cast<UBlackoutMinionHealthBarWidget>(WidgetComponent->GetUserWidgetObject()))
	{
		HealthBarWidget->SetForceHidden(true);
	}

	WidgetComponent->SetVisibility(false);
}

void UBOMinionHealthBarComponent::ApplyLayout()
{
	if (!WidgetComponent)
	{
		return;
	}

	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, HeightOffset));
	WidgetComponent->SetDrawSize(FVector2D(
		FMath::Max(DrawSize.X, 1.0f),
		FMath::Max(DrawSize.Y, 1.0f)));
}
