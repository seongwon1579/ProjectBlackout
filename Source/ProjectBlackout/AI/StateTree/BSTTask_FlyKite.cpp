#include "AI/StateTree/BSTTask_FlyKite.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include  "GameFramework/Pawn.h"
#include  "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"

EStateTreeRunStatus FBSTTask_FlyKite::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	const UWorld* World =  Context.GetWorld();
	const float Now = World ? World -> GetTimeSeconds() :0.0f;
	
	Data.bOrbitRight = FMath::RandBool();
	Data.NextReverseTime = Now + FMath::RandRange(Data.ReverseIntervalMin, Data.ReverseIntervalMax);
	
	if (const ACharacter* Character = Cast<ACharacter>(Data.Controller ? Data.Controller->GetPawn() : nullptr))
	{
		if (Character->GetMesh())
		{
			Data.DefaultMeshRelRot = Character->GetMesh()->GetRelativeRotation();
		}
	}
	Data.CurrentBank = 0.0f;
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FBSTTask_FlyKite::Tick(FStateTreeExecutionContext& Context,
                                           const float DeltaTime) const
{
	
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	
	APawn* Pawn  =Data.Controller ? Data.Controller->GetPawn() : nullptr;
	const UWorld* World = Context.GetWorld();
	if (!Pawn || !Data.Target)
	{
		return EStateTreeRunStatus::Running;
	}
	

	const float Now = World->GetTimeSeconds();
	const FVector PawnLoc = Pawn->GetActorLocation();
	const FVector TargetLoc = Data.Target->GetActorLocation();
	
	// 수평 
	FVector FlatDelta = TargetLoc - PawnLoc;
	FlatDelta.Z =0.0f;
	const float Dist2D = FlatDelta.Size();
	const FVector RadialDir = FlatDelta.GetSafeNormal();
	
	// 가변 반경
	const float CurRange = Data.DesireRange + Data.RadiusVarAmplitude * FMath::Sin(Now * Data.RadiusVarFrequency);
	
	// 반경 유지 
	const float RangeError = Dist2D - CurRange;
	const FVector RadialInput = RadialDir * FMath::Clamp(RangeError / Data.RangeTolerance , -1.0f , 1.0f);
	
	// 궤도 선회
	if (Now>= Data.NextReverseTime)
	{
		Data.bOrbitRight =!Data.bOrbitRight;
		Data.NextReverseTime = Now + FMath::RandRange(Data.ReverseIntervalMin, Data.ReverseIntervalMax);
	}
	const FVector TangentDir = FVector::CrossProduct(FVector::UpVector , RadialDir) * (Data.bOrbitRight? 1.0f : -1.0f);
	const FVector TangentInput = TangentDir * Data.OrbitStrength;
	
	// 수직 ( 목표 고도 + bob )
	const float DesiredZ = TargetLoc.Z + Data.HeightOffset + Data.BobAmplitude * FMath::Sin(Now * Data.BobFrequency);
	const float VertError = DesiredZ  -PawnLoc.Z;
	const FVector VertInput = FVector::UpVector * FMath::Clamp(VertError / Data.VerticalTolerance, -1.0f, 1.0f);
	
	// 합성 
	const FVector MoveInput  = (RadialInput + TangentInput +VertInput).GetClampedToMaxSize(1.0f);
	Pawn->AddMovementInput(MoveInput);
	
	if (ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			const float TargetBank =(Data.bOrbitRight ? 1.0f : -1.0f) * Data.MaxBankAngle;
			Data.CurrentBank = FMath::FInterpTo(Data.CurrentBank,TargetBank,DeltaTime , Data.BankInterpSpeed);
			
			const FQuat BankQuat(Pawn->GetActorForwardVector(), FMath::DegreesToRadians(Data.CurrentBank));
			const FQuat BaseWorld = Pawn->GetActorQuat() * Data.DefaultMeshRelRot.Quaternion();
			Mesh->SetWorldRotation(BankQuat * BaseWorld);
		}
	}
	
	return EStateTreeRunStatus::Running;
}

void FBSTTask_FlyKite::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData(*this);
	if (const ACharacter* Character = Cast<ACharacter>(Data.Controller ? Data.Controller->GetPawn() : nullptr))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			Mesh->SetRelativeRotation(Data.DefaultMeshRelRot);
		}
	}
}
