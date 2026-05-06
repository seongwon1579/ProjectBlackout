#include "AI/BehaviorTree/Tasks/BTTask_TurnToTarget.h"
#include "AI/BehaviorTree/BTNodeHelper.h"
#include "AI/BOAICalcHelper.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
	NodeName = "Turn To Target";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACharacter* Char   = Cast<ACharacter>(UBTNodeHelper::GetAIPawn(OwnerComp));
	AActor*     Target = UBTNodeHelper::GetActorFromBB(OwnerComp, TargetKey);
	if (!Char || !Target) return EBTNodeResult::Failed;

	float AngleDelta = 0.f;
	const EBOTurnDirection Dir = UBOAICalcHelper::GetTurnDirection(Char, Target, TurnThreshold, AngleDelta);

	if (Dir == EBOTurnDirection::None)
	{
		return EBTNodeResult::Succeeded;
	}

	UAnimMontage* Montage = (Dir == EBOTurnDirection::Left) ? TurnLeftMontage : TurnRightMontage;
	if (!Montage) return EBTNodeResult::Failed;

	// AI별 인스턴스 메모리를 TurnMemory 구조체로 해석하여 접근한다.
	FBTTask_TurnMemory* Mem = reinterpret_cast<FBTTask_TurnMemory*>(NodeMemory);

	// 몽타주 루트모션이 회전을 제어하도록 이동 회전을 잠근다.
	if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
	{
		Mem->bWasOrientToMovement          = Move->bOrientRotationToMovement;
		Move->bOrientRotationToMovement = false;
	}

	Char->PlayAnimMontage(Montage);
	Mem->ActiveMontage = Montage;

	return EBTNodeResult::InProgress;
}

void UBTTask_TurnToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	ACharacter* Char = Cast<ACharacter>(UBTNodeHelper::GetAIPawn(OwnerComp));
	if (!Char) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	FBTTask_TurnMemory* Mem      = reinterpret_cast<FBTTask_TurnMemory*>(NodeMemory);
	UAnimInstance*      AnimInst = Char->GetMesh() ? Char->GetMesh()->GetAnimInstance() : nullptr;

	if (!AnimInst || !AnimInst->Montage_IsPlaying(Mem->ActiveMontage))
	{
		RestoreMovement(Char, Mem->bWasOrientToMovement);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_TurnToTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACharacter* Char = Cast<ACharacter>(UBTNodeHelper::GetAIPawn(OwnerComp));
	if (Char)
	{
		FBTTask_TurnMemory* Mem = reinterpret_cast<FBTTask_TurnMemory*>(NodeMemory);
		Char->StopAnimMontage(Mem->ActiveMontage);
		RestoreMovement(Char, Mem->bWasOrientToMovement);
	}
	return EBTNodeResult::Aborted;
}

void UBTTask_TurnToTarget::RestoreMovement(ACharacter* Char, bool bOrientToMovement) const
{
	if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = bOrientToMovement;
	}
}