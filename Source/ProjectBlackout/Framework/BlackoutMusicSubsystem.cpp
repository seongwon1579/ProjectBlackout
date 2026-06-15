#include "Framework/BlackoutMusicSubsystem.h"

#include "Framework/BlackoutAudioSettings.h"
#include "Framework/BlackoutGameState.h"
#include "Core/BlackoutLog.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundClass.h"
#include "TimerManager.h"

bool UBlackoutMusicSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer))
	{
		return false;
	}

	return !IsRunningDedicatedServer();
}

void UBlackoutMusicSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UBlackoutMusicSubsystem::HandlePostLoadMapWithWorld);
}

void UBlackoutMusicSubsystem::Deinitialize()
{
	UnbindFromGameState();
	PendingBindWorld.Reset();
	RemainingBindRetries = 0;

	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	StopCurrentMusic();

	Super::Deinitialize();
}

void UBlackoutMusicSubsystem::PlayMainMenuMusic()
{
	const UBlackoutAudioSettings* AudioSettings = UBlackoutAudioSettings::GetBlackoutAudioSettings();
	if (!AudioSettings)
	{
		return;
	}

	PlayConfiguredMusic(AudioSettings->MainMenuMusic, TEXT("MainMenu"));
}

void UBlackoutMusicSubsystem::PlayLobbyMusic()
{
	const UBlackoutAudioSettings* AudioSettings = UBlackoutAudioSettings::GetBlackoutAudioSettings();
	if (!AudioSettings)
	{
		return;
	}

	PlayConfiguredMusic(AudioSettings->LobbyMusic, TEXT("Lobby"));
}

void UBlackoutMusicSubsystem::PlayMusicAsset(const TSoftObjectPtr<USoundBase>& MusicAsset, FName TrackName)
{
	if (TrackName.IsNone())
	{
		const FString AssetName = MusicAsset.GetAssetName();
		TrackName = AssetName.IsEmpty() ? TEXT("DynamicMusic") : FName(*AssetName);
	}

	PlayConfiguredMusic(MusicAsset, TrackName);
}

void UBlackoutMusicSubsystem::StopCurrentMusic()
{
	if (!IsValid(MusicComponent))
	{
		CurrentTrackName = NAME_None;
		return;
	}

	MusicComponent->Stop();
	MusicComponent->DestroyComponent();
	MusicComponent = nullptr;
	CurrentTrackName = NAME_None;
}

void UBlackoutMusicSubsystem::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
	// 같은 프로세스의 다른 PIE/GameInstance 월드는 무시합니다.
	if (!LoadedWorld || LoadedWorld->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	UnbindFromGameState();

	// 월드가 바뀌면 이전 월드에 묶인 오디오 컴포넌트는 새로 생성하도록 비웁니다.
	if (IsValid(MusicComponent) && MusicComponent->GetWorld() != LoadedWorld)
	{
		StopCurrentMusic();
	}

	PendingBindWorld = LoadedWorld;
	RemainingBindRetries = MaxBindRetries;
	RetryBindToGameState();
}

void UBlackoutMusicSubsystem::RetryBindToGameState()
{
	UWorld* TargetWorld = PendingBindWorld.Get();
	if (!TargetWorld)
	{
		return;
	}

	if (ABlackoutGameState* GameState = TargetWorld->GetGameState<ABlackoutGameState>())
	{
		BoundGameState = GameState;
		GameState->OnMatchStateChanged.RemoveDynamic(this, &UBlackoutMusicSubsystem::HandleMatchStateChanged);
		GameState->OnMatchStateChanged.AddDynamic(this, &UBlackoutMusicSubsystem::HandleMatchStateChanged);

		PendingBindWorld.Reset();
		RemainingBindRetries = 0;

		// 이미 복제된 매치 상태가 있으면 즉시 반영합니다.
		HandleMatchStateChanged(GameState->CurrentMatchState);
		return;
	}

	if (RemainingBindRetries <= 0)
	{
		PendingBindWorld.Reset();
		return;
	}

	--RemainingBindRetries;
	TargetWorld->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &UBlackoutMusicSubsystem::RetryBindToGameState));
}

void UBlackoutMusicSubsystem::UnbindFromGameState()
{
	if (ABlackoutGameState* GameState = BoundGameState.Get())
	{
		GameState->OnMatchStateChanged.RemoveDynamic(this, &UBlackoutMusicSubsystem::HandleMatchStateChanged);
	}

	BoundGameState.Reset();
}

void UBlackoutMusicSubsystem::PlayConfiguredMusic(const TSoftObjectPtr<USoundBase>& MusicAsset, const FName TrackName)
{
	if (MusicAsset.IsNull())
	{
		BO_LOG_CORE(Verbose, "BGM 재생 건너뜀: %s 트랙이 아직 설정되지 않았습니다.", *TrackName.ToString());
		return;
	}

	if (IsCurrentTrackPlaying(TrackName))
	{
		return;
	}

	UWorld* PlaybackWorld = ResolvePlaybackWorld();
	if (!PlaybackWorld)
	{
		BO_LOG_CORE(Warning, "BGM 재생 실패: 재생할 월드를 찾지 못했습니다. Track=%s", *TrackName.ToString());
		return;
	}

	USoundBase* MusicSound = MusicAsset.LoadSynchronous();
	if (!MusicSound)
	{
		BO_LOG_CORE(Warning, "BGM 재생 실패: 사운드 에셋 로드에 실패했습니다. Track=%s", *TrackName.ToString());
		return;
	}

	const UBlackoutAudioSettings* AudioSettings = UBlackoutAudioSettings::GetBlackoutAudioSettings();
	const float FadeInDuration = AudioSettings
		? FMath::Max(0.0f, AudioSettings->BackgroundMusicFadeInDuration)
		: 0.0f;
	USoundClass* MusicSoundClass = AudioSettings
		? AudioSettings->MusicSoundClass.LoadSynchronous()
		: nullptr;

	if (!IsValid(MusicComponent) || MusicComponent->GetWorld() != PlaybackWorld)
	{
		StopCurrentMusic();

		MusicComponent = UGameplayStatics::CreateSound2D(
			PlaybackWorld,
			MusicSound,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			false,
			false);

		if (!IsValid(MusicComponent))
		{
			BO_LOG_CORE(Warning, "BGM 재생 실패: AudioComponent 생성에 실패했습니다. Track=%s", *TrackName.ToString());
			return;
		}
	}
	else
	{
		MusicComponent->Stop();
		MusicComponent->SetSound(MusicSound);
	}

	// 모든 BGM이 옵션 메뉴의 Music 볼륨 슬라이더 영향을 받도록 SoundClass를 강제합니다.
	MusicComponent->SoundClassOverride = MusicSoundClass;
	MusicComponent->bAllowSpatialization = false;
	MusicComponent->bIsUISound = false;
	MusicComponent->bAutoDestroy = false;

	if (FadeInDuration > KINDA_SMALL_NUMBER)
	{
		MusicComponent->FadeIn(FadeInDuration, 1.0f, 0.0f);
	}
	else
	{
		MusicComponent->Play(0.0f);
	}

	CurrentTrackName = TrackName;
}

UWorld* UBlackoutMusicSubsystem::ResolvePlaybackWorld() const
{
	if (UWorld* BoundWorld = PendingBindWorld.Get())
	{
		return BoundWorld;
	}

	if (ABlackoutGameState* GameState = BoundGameState.Get())
	{
		return GameState->GetWorld();
	}

	if (!GEngine)
	{
		return nullptr;
	}

	if (UWorld* CurrentPlayWorld = GEngine->GetCurrentPlayWorld())
	{
		return CurrentPlayWorld;
	}

	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (UWorld* World = WorldContext.World())
		{
			switch (WorldContext.WorldType)
			{
			case EWorldType::PIE:
			case EWorldType::Game:
			case EWorldType::GamePreview:
				return World;

			default:
				break;
			}
		}
	}

	return nullptr;
}

bool UBlackoutMusicSubsystem::IsCurrentTrackPlaying(const FName TrackName) const
{
	return CurrentTrackName == TrackName
		&& IsValid(MusicComponent)
		&& MusicComponent->IsPlaying();
}

void UBlackoutMusicSubsystem::HandleMatchStateChanged(const EBlackoutMatchState NewState)
{
	if (NewState == EBlackoutMatchState::ShelterPrep)
	{
		PlayLobbyMusic();
	}
}
