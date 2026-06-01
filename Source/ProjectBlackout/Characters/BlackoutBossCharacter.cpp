#include "BlackoutBossCharacter.h"
#include "BlackoutBaseAttributeSet.h"
#include "GAS/BlackoutAbilitySystemComponent.h"
#include "UI/BlackoutHUD.h"
#include "UI/BlackoutEnemyHUDWidgetController.h"

ABlackoutBossCharacter::ABlackoutBossCharacter()
{
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

void ABlackoutBossCharacter::BeginPlay()
{
	Super::BeginPlay();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC) return;
	
	SetData();
	
	if (HasAuthority())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			   UBlackoutBaseAttributeSet::GetHealthAttribute())
		   .AddUObject(this, &ABlackoutBossCharacter::OnDamageReceived);
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick(
		this, &ABlackoutBossCharacter::TryBindToHUD);
}

void ABlackoutBossCharacter::TryBindToHUD()
{
	bool bBound = false;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue;

		if (ABlackoutHUD* BlackoutHUD = PC->GetHUD<ABlackoutHUD>())
		{
			if (UBlackoutEnemyHUDWidgetController* EnemyHUDController = BlackoutHUD->GetEnemyHUDWidgetController())
			{
				EnemyHUDController->BindToEnemy(GetAbilitySystemComponent(), GetBossDisplayName());
				bBound = true;
			}
		}
	}

	// HUD가 아직 준비되지 않은 경우 다음 틱에 재시도한다.
	if (!bBound)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ABlackoutBossCharacter::TryBindToHUD);
	}
}

void ABlackoutBossCharacter::OnDeath()
{
	Super::OnDeath();
	if (HasAuthority())
	{
		OnDefeated.Broadcast();
	}
}
