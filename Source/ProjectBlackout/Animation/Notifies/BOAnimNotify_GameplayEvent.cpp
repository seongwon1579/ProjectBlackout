#include "Animation/Notifies/BOAnimNotify_GameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"

void UBOAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !EventTag.IsValid())
	{
		return;
	}

	if (AActor* Owner = MeshComp->GetOwner())
	{
		FGameplayEventData Payload;
		Payload.EventTag = EventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, Payload);
	}
}

FString UBOAnimNotify_GameplayEvent::GetNotifyName_Implementation() const
{
	return EventTag.IsValid() ? FString::Printf(TEXT("GameplayEvent: %s"), *EventTag.ToString()) : TEXT("GameplayEvent: (None)");
}
