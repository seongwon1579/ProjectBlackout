#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Core/BlackoutTypes.h"
#include "Engine/TimerHandle.h"
#include "BlackoutPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UBlackoutAbilitySystemComponent;
class UBlackoutCombatComponent;
class AActor;
class UBlackoutClassSelectWidget;
class UBlackoutClassSelectWidgetController;
class UBlackoutMainMenuWidget;
class ABlackoutCharacterPreviewManager;

UCLASS()
class PROJECTBLACKOUT_API ABlackoutPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ABlackoutPlayerController();

	virtual void BeginPlay() override;
	virtual void AcknowledgePossession(APawn* P) override;

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Blackout|Controller")
	void Server_SelectClass(FGameplayTag ClassTag);
	
	UFUNCTION(Server, Reliable , BlueprintCallable , Category= "Blackout|Controller")
	void Server_SetReady(bool bNewReady);
	
	UFUNCTION(Server, Reliable , WithValidation , Category="Blackout|Controller")
	void Server_SetPlayerDisplayName(const FString& InName);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Blackout|Controller")
	void Client_OpenClassSelectUI();

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Open Class Select UI"), Category = "Blackout|Controller")
	void ReceiveOpenClassSelectUI();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Blackout|Controller")
	void Client_ShowDamageNumber(float DamageAmount, bool bIsCritical);

	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="On Show Damage Number"), Category = "Blackout|Controller")
	void ReceiveShowDamageNumber(float DamageAmount, bool bIsCritical);

	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "Blackout|Controller")
	void Client_ShowDamageNumberAtLocation(float DamageAmount, FVector WorldLocation, bool bIsCritical);

	UFUNCTION(BlueprintCallable, Category = "Blackout|Controller")
	void EnterSpectatorMode();

	UFUNCTION(BlueprintCallable, Category = "Blackout|Controller")
	void ExitSpectatorMode();

	UFUNCTION(Client, Reliable, Category = "Blackout|Controller")
	void Client_SetSpectateTarget(AActor* TargetActor, float BlendTime);

	UFUNCTION(Client, Reliable, Category = "Blackout|Controller")
	void Client_ReturnToOwnPawnView(float BlendTime);

	/**
	 * 사망한 관전 플레이어가 A/D 입력으로 관전 대상을 순환할 때 서버에 요청합니다.
	 * 서버가 살아있는 아군 중 현재 대상의 이전/다음을 찾아 ViewTarget을 변경합니다.
	 * @param Direction -1=이전, +1=다음.
	 */
	UFUNCTION(Server, Reliable, Category = "Blackout|Controller|Spectator")
	void Server_CycleSpectateTarget(int32 Direction);

	/** 쓰러지거나 완전 사망한 상태에서 항복 투표 개시 요청 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Blackout|Controller|Surrender")
	void Server_RequestSurrenderVote();

	/** 개시된 항복 투표에 대해 찬성/반대 투표 행사 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Blackout|Controller|Surrender")
	void Server_CastSurrenderVote(bool bAgree);

	/** 항복 투표 활성화 시 동적으로 찬성/반대 조작 IMC를 푸시/팝 하기 위한 클라이언트 RPC */
	UFUNCTION(Client, Reliable, Category = "Blackout|Controller|Surrender")
	void Client_SetSurrenderInputContextActive(bool bActive);
	
	/** ESC 또는 OnSelectionConfirmed 후 자동 호출. Widget 정리 + IMC 원복. */
	UFUNCTION(BlueprintCallable,Category="Blackout|ClassSelect")
	void CloseClassSelectUI();

	/** 인게임 ESC 메뉴를 엽니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|UI")
	void OpenPlayerMenu();

	/** 인게임 ESC 메뉴를 닫고 게임 입력으로 복귀합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|UI")
	void ClosePlayerMenu();
	
	/** 레벨 전환 전 , 서버가 각 클라 화면을 페이드아웃. bHoldUntilReady=true 면 도착 후 ready 신호까지 fade-in 보류 */
	UFUNCTION(Client,Reliable , Category="Blackout|Controller|Transition")
	void Client_StartScreenFadeOut(FLinearColor FadeColor, bool bHoldUntilReady = false);
	
	/** 서버가 보스 전투 시작 시 각 클라에 통지 — readiness 게이트의 서버측 신호 */
	UFUNCTION(Client,Reliable , Category="Blackout|Controller|Transition")
	void Client_NotifyBossCombatReady();
	
private:
	/** 페이드 지속시간 */
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Transition")
	float ScreenFadeDuration = 1.2f;
	
	/** ready 판정 최소 홀드 시간(깜빡임 방지) */
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Transition")
	float MinReadinessHoldTime = 0.3f;
	
	/** ready 판정 상한 캡 — 초과 시 강제 fade-in(영영 idle 안 되는 환경 방어) */
	UPROPERTY(EditDefaultsOnly , Category="Blackout|Transition")
	float MaxReadinessHoldTime = 5.0f;
	
	/** 페이드 아웃에 쓴 색 보관 -> 도착후 페이드 인에 재사용 */
	FLinearColor LastFadeColor = FLinearColor::Black;
	
	/** 전환 페이드 진행 중 플래그, 도착시 페이드 인 여부 판정 */
	bool bScreenFadePending = false;
	
	/** 서버 전투시작 통지 수신 여부(readiness 서버측 신호) */
	bool bServerCombatReady = false;
	
	FTimerHandle ReadinessPollTimer;
	float ReadinessWaitStartTime = 0.f;
	
	/** 도착 후 화면 복귀 */
	void StartScreenFadeIn();

	/** seamless 도착 시 로딩 게이트 재커버+폴링 시작 시도(게이트 플래그 set + 카메라 준비 시). 처리하면 true, 카메라 미준비면 false(다음 콜백서 재시도). */
	bool TryBeginLoadingGate();

	/** hold 전환 도착 시 readiness 폴링 시작 */
	void BeginReadinessGatedFadeIn();

	/** 스트리밍 idle + 최소홀드 + 서버신호 충족 또는 캡 초과 시 fade-in */
	void PollLevelReadiness();
	
	/** 오너 클라이언트가 로그인 닉네임을 서버 PlayerState에 반영하도록 송신 */
	void SendDisplayNameToServer();

	/** 로컬 미리보기/서버 권위 공용 치트 플래그 적용 경로입니다. */
	void ApplyDebugCheatFlags(bool bNewInfiniteHealth, bool bNewInfiniteStamina, bool bNewInfiniteAmmo);

public:
	/** CheatManager가 서버에서 동일한 치트 명령 문자열을 실행하도록 전달하는 공용 RPC입니다. */
	UFUNCTION(Server, Reliable, WithValidation, Category = "Blackout|Cheat")
	void Server_RunCheatCommand(const FString& CheatCommand);

	/** CheatManager와 서버 RPC가 공유하는 로컬 치트 명령 실행 진입점입니다. */
	bool ExecuteCheatCommandLocally(const FString& CheatCommand);

	/** 로컬 화면에 스턴 게이지 디버그 정보를 주기적으로 표시합니다. */
	void SetStunGaugeDebugEnabled(bool bEnabled);

	/** 클래스 선택 UI 표시 상태에 맞춰 월드 카메라와 프리뷰 캡처를 전환합니다. */
	void SetClassSelectRenderingState(bool bActive);

	/** 스트리밍 레벨의 프리뷰 매니저가 늦게 로드될 때 렌더링 상태 적용을 재시도합니다. */
	void RetryApplyClassSelectRenderingState();

	/** 개발 빌드에서 로컬 PlayerController가 CheatManager를 확실히 보유하도록 보장합니다. */
	void EnsureCheatManagerReady();

	ABlackoutCharacterPreviewManager* FindCharacterPreviewManager() const;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> ClassSelectPreviousViewTarget;

	FTimerHandle ClassSelectRenderingRetryHandle;
	int32 ClassSelectRenderingRetryCount = 0;
	static constexpr int32 ClassSelectRenderingMaxRetries = 30;
	bool bClassSelectRenderingStateActive = false;

#pragma region InputSetup
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_PlayerState() override;
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	/** 사망 후 관전 상태일 때만 활성화되는 입력 매핑 컨텍스트입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Spectator")
	TObjectPtr<UInputMappingContext> SpectatorMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Spectator")
	TObjectPtr<UInputAction> SpectatePrevAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Spectator")
	TObjectPtr<UInputAction> SpectateNextAction;

	/** 항복 투표가 활성화되었을 때 로컬 플레이어에게 동적으로 밀어 넣는 입력 매핑 컨텍스트입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Surrender")
	TObjectPtr<UInputMappingContext> SurrenderVoteMappingContext;

	/** 항복 투표 발의(최초 요청)용 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Surrender")
	TObjectPtr<UInputAction> RequestSurrenderAction;

	/** 찬성 투표용 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Surrender")
	TObjectPtr<UInputAction> VoteYesAction;

	/** 반대 투표용 입력 액션 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|Surrender")
	TObjectPtr<UInputAction> VoteNoAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> SwapWeaponAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> DebugSelfDamageAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> UseConsumable1Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> UseConsumable2Action;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input")
	TObjectPtr<UInputAction> UseRelicAction;
	
	/** 캐릭터 선택 UI 활성 시에만 사용하는 입력 컨텍스트 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|ClassSelect")
	TObjectPtr<UInputMappingContext> ClassSelectMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectNextAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectPrevAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectConfirmAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|Input|ClassSelect")
	TObjectPtr<UInputAction> ClassSelectCancelAction;

	/** 인게임 ESC 메뉴를 생성할 위젯 클래스입니다. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blackout|UI")
	TSubclassOf<UBlackoutMainMenuWidget> PlayerMenuWidgetClass;
	
	/** UMG 위젯 클래스 WBP_ClassSelect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Blackout|ClassSelect")
	TSubclassOf<UBlackoutClassSelectWidget> ClassSelectWidgetClass;
	
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutClassSelectWidget> ClassSelectWidget;
	
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutClassSelectWidgetController> ClassSelectController;

	/** 현재 화면에 열려 있는 인게임 ESC 메뉴 인스턴스입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UBlackoutMainMenuWidget> ActivePlayerMenuWidget;

	void OnClassSelectNextPressed();
	void OnClassSelectPrevPressed();
	void OnClassSelectConfirmPressed();
	void OnClassSelectCancelPressed();
	void OnMenuTogglePressed();

	UFUNCTION()
	void HandleClassSelectionConfirmed();

	UFUNCTION()
	void HandlePlayerMenuClosed();

	void OnFirePressed();
	void OnFireReleased();
	void OnAimPressed();
	void OnAimReleased();
	void OnReloadPressed();
	void OnSwapWeaponPressed();
	void OnDodgePressed();
	void OnSprintPressed();
	void OnSprintReleased();
	void OnDebugSelfDamagePressed();
	void OnInteractPressed();
	void OnInteractReleased();
	void OnUseConsumable1Pressed();
	void OnUseConsumable1Released();
	void OnUseConsumable2Pressed();
	void OnUseConsumable2Released();
	void OnUseRelicPressed();
	void OnUseRelicReleased();
	void OnSpectatePrevPressed();
	void OnSpectateNextPressed();
	void OnVoteYesPressed();
	void OnVoteNoPressed();
	void OnRequestSurrenderPressed();

	/** 관전 진입/이탈 시 SpectatorMappingContext를 푸시/팝합니다. */
	void SetSpectatorInputContextActive(bool bActive);
	
	bool IsHitReactInputBlocked() const;

	void HandleAbilityInputPressed(EBlackoutAbilityInputID InputID);
	void HandleAbilityInputReleased(EBlackoutAbilityInputID InputID);
	void TryInitHUD() const;
	UBlackoutAbilitySystemComponent* GetBlackoutAbilitySystemComponent() const;
	UBlackoutCombatComponent* GetBlackoutCombatComponent() const;
	void HandleStunGaugeDebugTick();
	void ClearStunGaugeDebugMessage() const;

	/** 로컬 화면에 띄우는 스턴 게이지 디버그 갱신 타이머입니다. */
	FTimerHandle StunGaugeDebugTimerHandle;

	/** 로컬 스턴 게이지 디버그 표시 활성 여부입니다. */
	bool bStunGaugeDebugEnabled = false;

#pragma endregion 
	
};
