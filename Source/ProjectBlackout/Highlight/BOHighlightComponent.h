// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: Custom Depth 스텐실로 진영/상태별 외곽선을 칠하고 로컬 조종 폰은 외곽선을 제외하는 하이라이트 컴포넌트
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Highlight/BlackoutStencil.h"
#include "BOHighlightComponent.generated.h"

class APawn;
class AController;

/**
 * 액터 메시에 Custom Depth 스텐실을 칠해 진영/상태별 외곽선을 표시하는 컴포넌트
 */
UCLASS(ClassGroup=(Blackout), meta=(BlueprintSpawnableComponent))
class PROJECTBLACKOUT_API UBOHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UBOHighlightComponent();
	
	/** 진영 외곽선 색 , 생성자 또는 BP 에서 지정 */
	UPROPERTY(EditDefaultsOnly , BlueprintReadOnly , Category="Blackout|Highlight")
	EBlackoutStencil BaseStencil = EBlackoutStencil::None;
	
	/** true 면 로컬 조종 폰일떄 외곽선 off ( 본인 제외용 )*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly , Category="Blackout|Highlight")
	bool bSuppressForLocalViewer  =false;

protected:
	
	/** 외곽선 대상 */
	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> TargetMesh;
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** 현재 상태(BaseStencil + 로컬제외) 를 TargetMesh의 CustomDepth에 반영 */
	void ApplyStencil();
	
	/** 소유 폰 컨트룰러 변경 시 로컬 조종 여부를 판정해 갱신 */
	UFUNCTION()
	void HandleControllerChanged(APawn* Pawn,AController* OldController , AController* NewController);

	

};
