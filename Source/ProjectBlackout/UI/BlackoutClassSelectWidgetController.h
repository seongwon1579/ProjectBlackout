// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: 클래스 선택 데이터 제공·3D 프리뷰·싱글 모드 연동 컨트롤러 구현
//  - 김민영: 산탄 무기 펠릿 수 조회 기능 추가
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SubclassOf.h"
#include "UI/BlackoutClassSelectTypes.h"
#include "Engine/TimerHandle.h"
#include "BlackoutClassSelectWidgetController.generated.h"

class APlayerController;
class UBOCharacterRoster;
class ABOFirearm;
class ABlackoutCharacterPreviewManager;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
FBlackoutClassSelectionChangedSignature,
const FBlackoutClassSelectDisplayData&, DisplayData
);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBlackoutClassSelectionConfirmedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
FBlackoutPreviewRenderTargetReadySignature,
UTextureRenderTarget2D*, RenderTarget
);


/**
 * 캐릭터 선택 UI 컨트롤러
 *  A/D → 인덱스 ±1 + Display broadcast (로컬). F → Server_SelectClass + UI 닫기 broadcast
 */
UCLASS(BlueprintType)
class PROJECTBLACKOUT_API UBlackoutClassSelectWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	bool Initialize(APlayerController* InPlayerController , const UBOCharacterRoster* InRoster);
	
	/** 현재 인덱스의 Display 를 다시 broadcast. Widget 초기 표시 시 호출 */
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void BroadcastCurrentSelection();
	
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void NavigateNext(); 
	
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void NavigatePrevious();
	
	/** F 확정 , PC->Server_SelectClass 호출 , OnSelectionConfirmed broadcast */
	UFUNCTION(BlueprintCallable, Category="Blackout|ClassSelect")
	void ConfirmSelection();
	
	UPROPERTY(BlueprintAssignable, Category="Blackout|ClassSelect")
	FBlackoutClassSelectionChangedSignature OnSelectionChanged;
	
	UPROPERTY(BlueprintAssignable, Category="Blackout|ClassSelect")
	FBlackoutClassSelectionConfirmedSignature OnSelectionConfirmed;

	/** Manager 의 client-local RT 가 준비되면 한 번 broadcast. Widget 이 Image_Portrait 에 SetBrushFromTexture */
	UPROPERTY(BlueprintAssignable, Category="Blackout|Preview")
	FBlackoutPreviewRenderTargetReadySignature OnPreviewRenderTargetReady;

	virtual void BeginDestroy() override;
	
private:
	FBlackoutClassSelectDisplayData BuildDisplayData(int32 Index) const;
	FBlackoutFirearmStat LookupFirearmStat(TSubclassOf<ABOFirearm> WeaponClass) const;
	bool LookupShotgunPelletCount(TSubclassOf<ABOFirearm> WeaponClass, int32& OutPelletCount) const;
	
	/** SubLevel 안 Manager 찾기 , 현재 인덱스 캐릭터 Pawn 갱신 첫 호출 시 GetAllActorsOfClass 로 lookup, 이후 캐싱 활용 */
	void UpdatePreviewPawn();
	
	TWeakObjectPtr<APlayerController> PlayerController;
	TWeakObjectPtr<const UBOCharacterRoster> Roster;
	TWeakObjectPtr<class ABlackoutCharacterPreviewManager> PreviewManager;
	int32 CurrentIndex = 0;
	bool bRenderTargetBroadcast = false;

	// 프리뷰 매니저(스트리밍 서브레벨 L_CharacterPreview)가 아직 로드 전일 때 재시도용.
	FTimerHandle PreviewManagerRetryHandle;
	int32 PreviewManagerRetryCount = 0;
	static constexpr int32 PreviewManagerMaxRetries = 30; // 0.1s × 30 ≈ 3초
};
