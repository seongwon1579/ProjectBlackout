// ─── 구현 내역 ───────────────────────
//  - 김민영: 항복·빠른 재도전 투표 위젯 구현
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutSurrenderVoteWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UBlackoutHUDWidgetController;

/**
 * 개별 플레이어의 투표 상태
 */
UENUM(BlueprintType)
enum class EBlackoutVoteChoice : uint8
{
	Unvoted   UMETA(DisplayName = "Unvoted"), // 미투표
	Yes       UMETA(DisplayName = "Yes"),     // 찬성
	No        UMETA(DisplayName = "No")       // 반대
};

/**
 * 블루프린트 UI 렌더링에 필요한 플레이어별 투표 의사 정보 구조체
 */
USTRUCT(BlueprintType)
struct FBlackoutPlayerVoteStatus
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Surrender")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Blackout|HUD|Surrender")
	EBlackoutVoteChoice VoteChoice = EBlackoutVoteChoice::Unvoted;
};

/**
 * 화면 좌측에 항복 투표 실시간 상태(LoL 스타일의 개별 이미지 변경용 데이터 제공, 시간 제한)를 표시하는 C++ 위젯입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTBLACKOUT_API UBlackoutSurrenderVoteWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 위젯 컨트롤러를 설정하고 델리게이트를 바인딩합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|HUD|Surrender")
	void SetWidgetController(UBlackoutHUDWidgetController* InWidgetController);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** GameState의 복제 데이터를 수신하여 실시간 UI 요소들을 갱신하는 콜백 */
	UFUNCTION()
	void HandleSurrenderVoteStateChanged(bool bIsActive, int32 YesCount, int32 NoCount, float EndTimeSeconds);

	/** 
	 * 블루프린트에서 수신하여 LoL 스타일로 개별 유저 이미지 도트의 색상을 변경하고 visual 연출을 처리하는 핵심 이벤트
	 * @param VoteStatuses 접속한 모든 플레이어의 이름 및 투표 선택 현황 배열 (블루프린트 루프로 이미지 동적 색상 변경 가능)
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Surrender Vote State Changed"), Category = "Blackout|HUD|Surrender")
	void ReceiveSurrenderVoteStateChanged(bool bIsActive, const TArray<FBlackoutPlayerVoteStatus>& VoteStatuses, float EndTimeSeconds);

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Surrender")
	bool bIsVoteActive = false;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Blackout|HUD|Surrender")
	float VoteEndTimeSeconds = 0.f;

	/** 남은 제한 시간 텍스트 (바인딩 선택) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Surrender")
	TObjectPtr<UTextBlock> TimerText;

	/** 제한 시간 감소 비율 프로그레스 바 (바인딩 선택) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Blackout|HUD|Surrender")
	TObjectPtr<UProgressBar> TimerProgressBar;

private:
	void UpdateTimerDisplay();
};
