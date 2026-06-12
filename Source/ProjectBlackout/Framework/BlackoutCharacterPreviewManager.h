// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlackoutCharacterPreviewManager.generated.h"

class USceneCaptureComponent2D;
class UCameraComponent;
/**
 * 캐릭터 선택 UI 의 3D Preview Pawn 라이프사이클 관리.
 * L_CharacterPreview Sublevel 안 한 개 배치. 위치 = Pawn spawn 위치.
 * WidgetController 가 인덱스 변경 시 SetPreviewCharacter 호출.
 */
UCLASS()
class PROJECTBLACKOUT_API ABlackoutCharacterPreviewManager : public AActor
{
	GENERATED_BODY()

public:
	
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintCallable , Category="Blackout|Preview")
	UTextureRenderTarget2D* GetRenderTarget() const {return DynamicRT; }
	
	// Sets default values for this actor's properties
	ABlackoutCharacterPreviewManager();
	
	/** 현재 Preview Actor 교체. nullptr 이면 destroy, 같은 클래스면 무시 */
	UFUNCTION(BlueprintCallable , Category="Blackout|Preview")
	void SetPreviewCharacter(TSubclassOf<AActor> PreviewClass);
	
	/** Preview Pawn destroy + 참조 정리. UI 닫힐 때 호출. */
	UFUNCTION(BlueprintCallable , Category="Blackout|Preview")
	void ClearPreview();

	/** 클래스 선택 UI 표시 여부에 맞춰 프리뷰 SceneCapture 를 켜고 끈다. */
	UFUNCTION(BlueprintCallable, Category="Blackout|Preview")
	void SetPreviewCaptureActive(bool bActive);

protected:
	UPROPERTY(VisibleAnywhere, Category="Blackout|Preview")
	TObjectPtr<USceneComponent> SpawnRoot;

	/** 클래스 선택 UI 활성 중 플레이어 ViewTarget 으로 사용할 경량 카메라. */
	UPROPERTY(VisibleAnywhere, Category="Blackout|Preview")
	TObjectPtr<UCameraComponent> ViewCamera;
	
	/** spawn 시 Pawn의 Yaw 회전 */
	UPROPERTY(EditAnywhere , BlueprintReadWrite , Category="Blackout|Preview")
	float PreviewYawOffset = 180.0f;
	
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentPreviewActor;

	UPROPERTY(Transient)
	TSubclassOf<AActor> CurrentPreviewClass;
	
	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> CaptureComp;
	
	UPROPERTY(EditAnywhere , Category="Blackout|Preview")
	int32 RTSizeX = 768;
	
	UPROPERTY(EditAnywhere, Category="Blackout|Preview")
	int32 RTSizeY = 800;
	
	UPROPERTY(Transient)
	TObjectPtr<class UTextureRenderTarget2D> DynamicRT;
	
};
