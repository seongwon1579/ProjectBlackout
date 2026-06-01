#include "Animation/Notifies/BOAnimNotify_GameplayCue.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Combat/BlackoutWeaponCueLibrary.h"
#include "Components/SkeletalMeshComponent.h"

UBOAnimNotify_GameplayCue::UBOAnimNotify_GameplayCue()
{
}

void UBOAnimNotify_GameplayCue::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !GameplayCueTag.IsValid())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// 소유 액터의 ASC(AbilitySystemComponent) 획득
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
	{
		return;
	}

	// GameplayCue 실행을 위한 공통 파라미터 빌드
	FGameplayCueParameters Params;
	Params.Location = Owner->GetActorLocation();
	Params.Normal = Owner->GetActorForwardVector();
	Params.Instigator = Owner->GetInstigator();
	Params.EffectCauser = Owner;
	Params.SourceObject = Owner;

	// Cue 라이브러리를 통해 동기화 실행
	UBlackoutWeaponCueLibrary::ExecuteWeaponCue(ASC, GameplayCueTag, Params);
}

FString UBOAnimNotify_GameplayCue::GetNotifyName_Implementation() const
{
	if (GameplayCueTag.IsValid())
	{
		return FString::Printf(TEXT("GameplayCue: %s"), *GameplayCueTag.ToString());
	}
	return TEXT("GameplayCue: (None)");
}
