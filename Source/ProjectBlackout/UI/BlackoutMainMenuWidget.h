// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BlackoutMainMenuWidget.generated.h"

class UButton;
class UTextBlock;
class UBlackoutLoginWidget;
class UBlackoutMatchmakingWidget;
class UBlackoutSettingsWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlackoutMainMenuClosed);

/**
 *  메인메뉴 위젯 
 *  비로그인 : [로그인] / [매칭 시작] 비활성 / [옵션?] / [종료]
 *  로그인됨 : Welcome(나중에 메인 화면 으로 변경) / [로그아웃] / [매칭 시작] / [옵션] / [종료]
 *  
 *  - [로그인]       → LoginWidget 모달 표시
 *  - [매칭 시작]    → (후속) Matchmaking 위젯 표시
 *  - [로그아웃]     → Subsystem->Logout() + 상태 갱신
 *  - 401 응답 수신  → OnMatchmakingError 구독으로 자동 로그아웃 처리 
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// WBP_Login 등 구체 구현은 BP 에서 지정.
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Blackout|MainMenu")
	TSubclassOf<UBlackoutLoginWidget> LoginWidgetClass;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Blackout|MainMenu")
	TSubclassOf<UBlackoutMatchmakingWidget> MatchmakingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|MainMenu")
	TSubclassOf<UBlackoutSettingsWidget> SettingsWidgetClass;
	
	/** 혼자하기 진입시 OpenLevel 로비맵 **/
	UPROPERTY(EditDefaultsOnly , BlueprintReadWrite, Category = "Blackout|MainMenu")
	TSoftObjectPtr<UWorld> SinglePlayLobbyMap;

	/** 인게임에서 ESC 메뉴로 사용할 때 활성화하는 모드입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackout|MainMenu")
	bool bUseAsInGameMenu = false;

	/** 인게임 메뉴가 닫힐 때 플레이어 컨트롤러가 입력 모드를 복구할 수 있도록 알립니다. */
	UPROPERTY(BlueprintAssignable, Category = "Blackout|MainMenu")
	FOnBlackoutMainMenuClosed OnMenuClosed;

	UFUNCTION(BlueprintCallable, Category = "Blackout|MainMenu")
	void CloseMenu();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LoginButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LogoutButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> StartMatchmakingButton;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> SinglePlayButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> OptionsButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> BackButton;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> WelcomeText;
	
private:
	UFUNCTION()
	void HandleLoginClicked();
	
	UFUNCTION()
	void HandleLogoutClicked();
	
	UFUNCTION()
	void HandleStartMatchmakingClicked();
	
	UFUNCTION()
	void HandleSinglePlayClicked();
	
	UFUNCTION()
	void HandleOptionsClicked();
	
	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleBackClicked();

	UFUNCTION()
	void HandleSettingsClosed();
	
	UFUNCTION()
	void HandleLoginAttemptFinished(bool bSuccess , const FString& PlayerName);
	
	UFUNCTION()
	void HandleMatchmakingError(int32 HttpStatus , const FString& Message);
	
	UFUNCTION()
	void HandleMatchmakingWidgetExited(bool bSuccess);
	
	// 로그인 상태에 맞춰서 버튼 Visible / Welcome 텍스트 갱신
	void RefreshForLoginState();
	
	UPROPERTY()
	TObjectPtr<UBlackoutLoginWidget> ActiveLoginWidget;
	
	UPROPERTY()
	TObjectPtr<UBlackoutMatchmakingWidget> ActiveMatchmakingWidget;

	UPROPERTY()
	TObjectPtr<UBlackoutSettingsWidget> ActiveSettingsWidget;
};
