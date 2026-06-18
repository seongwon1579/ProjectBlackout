// ─── 구현 내역 ───────────────────────
//  - 허혁: 로컬 BGM 서브시스템 — 타이틀/로비 트랙 자동 전환·중복 재생 방지·GameState 매치 상태 구독
// ──────────────────────────────────────

#pragma once

#include "CoreMinimal.h"
#include "Core/BlackoutTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "BlackoutMusicSubsystem.generated.h"

class ABlackoutGameState;
class UAudioComponent;
class USoundBase;
class UWorld;

/**
 * 타이틀/로비처럼 레벨 전환에 따라 바뀌는 간단한 BGM 재생을 담당하는 로컬 전용 서브시스템입니다.
 */
UCLASS()
class PROJECTBLACKOUT_API UBlackoutMusicSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 전용 서버에서는 로컬 음악 재생이 필요 없으므로 서브시스템을 생성하지 않습니다. */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 타이틀 메인 메뉴 기본 BGM을 재생합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void PlayMainMenuMusic();

	/** ShelterPrep 로비 기본 BGM을 재생합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void PlayLobbyMusic();

	/** 외부 액터가 지정한 임의의 음악 에셋을 현재 BGM 슬롯으로 재생합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void PlayMusicAsset(const TSoftObjectPtr<USoundBase>& MusicAsset, FName TrackName = NAME_None, float FadeInDurationOverride = -1.0f);

	/** 현재 BGM을 페이드아웃한 뒤 다른 음악으로 자연스럽게 전환합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void TransitionToMusicAsset(const TSoftObjectPtr<USoundBase>& MusicAsset, float FadeOutDuration = 0.0f, FName TrackName = NAME_None, float FadeInDurationOverride = -1.0f);

	/** 현재 재생 중인 BGM을 페이드아웃만 수행하고 종료합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void FadeOutCurrentMusic(float FadeOutDuration = 0.0f);

	/** 현재 재생 중인 BGM을 즉시 정지합니다. */
	UFUNCTION(BlueprintCallable, Category = "Blackout|Audio")
	void StopCurrentMusic();

private:
	/** 맵 로드 직후 GameState를 다시 바인딩해 로비 진입을 감지합니다. */
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);

	/** 아직 GameState가 준비되지 않은 월드에 대해 다음 틱에 재시도합니다. */
	void RetryBindToGameState();

	/** 현재 월드의 GameState 델리게이트를 해제합니다. */
	void UnbindFromGameState();

	/** 메뉴/로비 트랙을 공통 경로로 재생합니다. */
	void PlayConfiguredMusic(const TSoftObjectPtr<class USoundBase>& MusicAsset, const FName TrackName, float FadeInDurationOverride = -1.0f);

	/** 로컬 재생에 쓸 게임 월드를 찾습니다. */
	UWorld* ResolvePlaybackWorld() const;

	/** 같은 트랙이 이미 살아 있는 오디오 컴포넌트에서 재생 중인지 확인합니다. */
	bool IsCurrentTrackPlaying(const FName TrackName) const;

	/** 로비 매치 상태가 들어오면 자동으로 로비 BGM을 시작합니다. */
	UFUNCTION()
	void HandleMatchStateChanged(EBlackoutMatchState NewState);

	/** 맵 로드 델리게이트 핸들입니다. */
	FDelegateHandle PostLoadMapHandle;

	/** 다음 틱 재시도에 사용할 월드 약한 참조입니다. */
	TWeakObjectPtr<UWorld> PendingBindWorld;

	/** 현재 매치 상태 변경을 구독 중인 GameState입니다. */
	TWeakObjectPtr<ABlackoutGameState> BoundGameState;

	/** 지연 전환 타이머를 걸어둔 월드입니다. */
	TWeakObjectPtr<UWorld> TransitionTimerWorld;

	/** 페이드아웃 뒤 재생할 다음 트랙 정보입니다. */
	TSoftObjectPtr<USoundBase> PendingTransitionMusic;
	FName PendingTransitionTrackName = NAME_None;
	float PendingTransitionFadeInDurationOverride = -1.0f;

	/** 현재 활성 BGM을 재생 중인 2D 오디오 컴포넌트입니다. */
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MusicComponent = nullptr;

	/** 이미 재생 중인 트랙을 중복 시작하지 않기 위한 식별자입니다. */
	FName CurrentTrackName = NAME_None;

	/** GameState 재바인딩 최대 재시도 횟수입니다. */
	int32 RemainingBindRetries = 0;

	/** 현재 BGM 페이드아웃 후 다른 트랙으로 넘길 때 쓰는 타이머입니다. */
	FTimerHandle MusicTransitionTimerHandle;

	/** 타이머 만료 후 저장해 둔 다음 트랙 전환을 실행합니다. */
	void ExecutePendingMusicTransition();

	static constexpr int32 MaxBindRetries = 30;
};
