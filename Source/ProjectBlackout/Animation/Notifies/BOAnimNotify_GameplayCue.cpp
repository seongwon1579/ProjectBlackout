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

	FVector Location = Owner->GetActorLocation();
	FVector Normal = Owner->GetActorForwardVector();
	FQuat BaseRotation = Owner->GetActorQuat();

	// 소켓이 지정되어 있고 유효하다면 소켓의 트랜스폼을 사용
	if (SocketName != NAME_None && MeshComp->DoesSocketExist(SocketName))
	{
		Location = MeshComp->GetSocketLocation(SocketName);
		BaseRotation = MeshComp->GetSocketQuaternion(SocketName);
		Normal = BaseRotation.GetForwardVector();
	}

	// 로컬 위치 오프셋 적용
	if (!LocationOffset.IsZero())
	{
		Location += BaseRotation.RotateVector(LocationOffset);
	}

	// 회전 오프셋 적용
	if (!RotationOffset.IsZero())
	{
		Normal = RotationOffset.Quaternion().RotateVector(Normal);
	}

	// GameplayCue 실행을 위한 공통 파라미터 빌드
	FGameplayCueParameters Params;
	Params.Location = Location;
	Params.Normal = Normal;
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
