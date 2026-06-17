// Fill out your copyright notice in the Description page of Project Settings.

// ─── 구현 내역 ───────────────────────
//  - 최승현: 로그인 모달 위젯 구현 (시연용 단순 로그인 처리)
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutLoginWidget.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlackoutLoginAttemptFinished, bool,bSuccess,const FString&, PlayerName);

/**
 *  로그인 모달 위젯,
 *  UBlackoutMatchmakingSubsystem-> Login 트리거 , OnLoginComplete 결과
 *  메인 메뉴는 OnLoginAttemptFinished 구독해서 상태 갱신
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutLoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintAssignable, Category="Blackout|Login")
	FOnBlackoutLoginAttemptFinished  OnLoginAttemptFinished;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UEditableTextBox> PlayerNameInput;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LoginButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> CancelButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ErrorMessageText;
	
private:
	UFUNCTION()
	void HandleLoginClicked();
	
	UFUNCTION()
	void HandleCancelClicked();
	
	UFUNCTION()
	void HandleLoginComplete(bool bSuccess);
	
	// 요청 중 LoginButton/Cancel 비활성 , ErrorMessage 클리어
	void SetBusy(bool bBusy);
};
