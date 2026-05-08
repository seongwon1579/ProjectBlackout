#include "Animation/AnimNotifyState_BossAttackSweep.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/BlackoutGameplayTags.h"

void UAnimNotifyState_BossAttackSweep::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Owner,
			BlackoutGameplayTags::Event_Enemy_Attack_OnCollision,
			EventData
		);
	}
}

void UAnimNotifyState_BossAttackSweep::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			Owner,
			BlackoutGameplayTags::Event_Enemy_Attack_OffCollision,
			EventData
		);
	}
}

FString UAnimNotifyState_BossAttackSweep::GetNotifyName_Implementation() const
{
	return TEXT("Boss Attack Sweep");
}
