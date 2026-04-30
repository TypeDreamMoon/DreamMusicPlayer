// Dream Music Player 后端代码

#include "Classes/DreamMusicPlayerComponent.h"

#include "DreamMusicPlayerDebugLog.h"

#include "Classes/DreamMusicPlayerExpansion.h"
#include "Classes/DreamMusicAudioManager.h"

#include "Components/AudioComponent.h"
#include "Engine/AssetManager.h"

/**
 *		[0] PlayMusic -> [1]
 *		[1] SetMusicData(GetNextMusicData or GetLastMusicData) -> [2]
 *		[2] OnMusicLoaded	-> [3]
 *		[3] StartMusic
 */

void UDreamMusicPlayerComponent::OnMusicLoaded(bool bPlay)
{
	DMP_LOG_LIFETIME_START

	if (IsValid(CurrentMusicData.CachedMusic))
	{
		SoundWave = CurrentMusicData.CachedMusic;
	}
	else
	{
		SoundWave = CurrentMusicData.Music.Get();
	}

	Cover = CurrentMusicData.Tag.CoverArt;

	AudioManager->Music_Changed(CurrentMusicData);
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

		Expansion->ChangeMusic(CurrentMusicData);
	}

	OnMusicDataChanged.Broadcast(CurrentMusicData);

	if (bPlay)
	{
		StartMusic();
	}

	DMP_LOG_LIFETIME_END
}

void UDreamMusicPlayerComponent::StartMusic()
{
	DMP_LOG_LIFETIME_START

	if (!CurrentMusicData.IsValid())
	{
		DMP_LOG(Error, TEXT("Current Music Data Is Not Valid !!!"))
		return;
	}

	// Initialize State
	CurrentMusicDuration = 0.0f;
	CurrentMusicPercent = 0.0f;
	CurrentDuration = 0.0f;
	LastSeekPosition = 0.0f;
	MusicStartWorldTime = FPlatformTime::Seconds(); // 记录开始时间
	bJustSeeked = false;
	CurrentTimestamp = FDreamMusicTimestamp();

	// Validate SoundWave before playing
	if (!SoundWave || !SoundWave->IsValidLowLevel())
	{
		if (CurrentMusicData.MusicType == EDreamMusicPlayerMusicType::Network)
		{
			DMP_LOG(Log, TEXT("Waiting for network music download..."));
		}
		else
		{
			DMP_LOG(Error, TEXT("Invalid SoundWave for music: %s"), *CurrentMusicData.Tag.Title);
		}
		return;
	}

	// Play Music with improved setup
	CurrentMusicDuration = SoundWave->Duration;

	AudioManager->Music_Start();

	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

		Expansion->MusicStart();
	}
	
	AudioManager->Music_Play();

	// Update state
	bIsPaused = false;
	bIsPlaying = true;
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Playing);

	// Callback
	OnMusicPlay.Broadcast(CurrentMusicData);
	DMP_LOG(Log, TEXT("Play Music : Name : %-15s Duration : %f"), *CurrentMusicData.Tag.Title, CurrentMusicDuration);
}

void UDreamMusicPlayerComponent::EndMusic(bool Native)
{
	DMP_LOG_LIFETIME_START

	if (!bIsPlaying)
	{
		return; // Already stopped
	}

	UAudioComponent* ActiveComponent = AudioManager->GetAudioComponent();
	if (!ActiveComponent)
	{
		DMP_LOG(Warning, TEXT("No active audio component to stop"));
		return;
	}

	AudioManager->Music_End();
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

		Expansion->MusicEnd();
	}

	// Update state immediately
	bIsPaused = false;
	bIsPlaying = false;
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Stop);

	// Clean up state
	CurrentDuration = 0.0f;
	CurrentMusicDuration = 0.0f;
	CurrentMusicPercent = 0.0f;

	OnMusicEnd.Broadcast();
	DMP_LOG(Log, TEXT("Music End : Name : %-15s Play Mode : %d"), *CurrentMusicData.Tag.Title, (int)PlayMode);

	// Handle auto-play logic only if not manually stopped
	if (!Native)
	{
		// Add small delay to ensure clean transition
		if (GWorld)
		{
			TWeakObjectPtr<UDreamMusicPlayerComponent> WeakThis(this);
			GWorld->GetTimerManager().SetTimerForNextTick([WeakThis]()
			{
				if (UDreamMusicPlayerComponent* StrongThis = WeakThis.Get())
				{
					StrongThis->HandleAutoPlayTransition();
				}
			});
		}
	}

	DMP_LOG_LIFETIME_END
}

void UDreamMusicPlayerComponent::HandleAutoPlayTransition()
{
	DMP_LOG_LIFETIME_START

	if (!bIsPlaying) // Ensure we're still in stopped state
	{
		switch (PlayMode)
		{
		case EDreamMusicPlayerPlayMode::EDMPPS_Loop:
			if (CurrentMusicData.IsValid())
			{
				SetMusicData(CurrentMusicData, true);
			}
			break;
		case EDreamMusicPlayerPlayMode::EDMPPS_Normal:
		case EDreamMusicPlayerPlayMode::EDMPPS_Random:
			PlayNextMusic();
			break;
		}
	}

	DMP_LOG_LIFETIME_END
}

void UDreamMusicPlayerComponent::PauseMusic()
{
	DMP_LOG_LIFETIME_START

	AudioManager->Music_Pause();

	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Paused);
	bIsPaused = true;

	// 暂停时保存当前精确时间，停止世界时间计算
	CurrentDuration = GetAccuratePlayTime();
	LastSeekPosition = CurrentDuration;
	MusicStartWorldTime = 0.0; // 停止世界时间基准

	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;
		Expansion->MusicPause();
	}

	OnMusicPause.Broadcast();

	DMP_LOG_LIFETIME_END
}

void UDreamMusicPlayerComponent::UnPauseMusic()
{
	DMP_LOG_LIFETIME_START

	AudioManager->Music_UnPause();

	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Playing);
	bIsPaused = false;

	// 恢复播放时重新设置时间基准
	MusicStartWorldTime = FPlatformTime::Seconds();
	bJustSeeked = true; // 标记为刚刚 Seek，使用保存的位置

	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

		Expansion->MusicUnPause();
	}

	OnMusicUnPause.Broadcast();

	DMP_LOG_LIFETIME_END
}

void UDreamMusicPlayerComponent::SetMusicData(FDreamMusicData InData, bool bPlay)
{
	DMP_LOG_LIFETIME_START

	CurrentMusicData = InData;

	if (CurrentMusicData.MusicType == EDreamMusicPlayerMusicType::Network)
	{
		// Network method

		for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
		{
			// 网络模式下 会调用 UDreamMusicPlayerExpansion_Network 加载音乐资源
			// 然后在 UDreamMusicPlayerExpansion_Network中会调用OnMusicLoaded函数 播放音乐
			if (Expansion != nullptr)
				Expansion->SetMusicData(InData);
		}
	}
	else
	{
		// Asset method

		for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
		{
			if (Expansion != nullptr)
				Expansion->SetMusicData(InData);
		}

		// 应对快速切换时
		// 如果当前正在加载中 则取消加载
		if (MusicLoadHandle.IsValid() && MusicLoadHandle->IsActive())
		{
			MusicLoadHandle->CancelHandle();
		}

		if (CurrentMusicData.Music.IsValid())
		{
			OnMusicLoaded(true);
		}
		else
		{
			MusicLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
				CurrentMusicData.Music.ToSoftObjectPath(),
				FStreamableDelegate::CreateUObject(this, &UDreamMusicPlayerComponent::OnMusicLoaded, bPlay));
		}
	}

	DMP_LOG_LIFETIME_END
}

void UDreamMusicPlayerComponent::SetPlayState(EDreamMusicPlayerPlayState InState)
{
	DMP_LOG_LIFETIME_START

	PlayState = InState;
	OnPlayStateChanged.Broadcast(PlayState);

	DMP_LOG_LIFETIME_END
}

void UDreamMusicPlayerComponent::MusicTick(float DeltaTime)
{
	float AccuratePlayTime = GetAccuratePlayTime();

	// 更新时间状态
	CurrentDuration = AccuratePlayTime;
	CurrentMusicPercent = FMath::Clamp(CurrentDuration / CurrentMusicDuration, 0.0f, 1.0f);
	CurrentTimestamp = *FDreamMusicTimestamp().FromSeconds(CurrentDuration);

	// Auto Next
	if (CurrentTimestamp >= CurrentMusicDuration)
	{
		EndMusic();
	}

	AudioManager->Tick(CurrentTimestamp, DeltaTime);
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

		Expansion->Tick(CurrentTimestamp, DeltaTime);
	}

	OnMusicTick.Broadcast(CurrentDuration);

	// 重置 Seek 标志
	if (bJustSeeked)
	{
		bJustSeeked = false;
	}
}


float UDreamMusicPlayerComponent::GetAccuratePlayTime() const
{
	if (!bIsPlaying || bIsPaused)
	{
		return CurrentDuration;
	}

	// 如果刚刚进行了 Seek，使用 Seek 位置作为基准
	if (bJustSeeked)
	{
		return LastSeekPosition;
	}

	// 使用世界时间来计算更精确的播放时间
	if (MusicStartWorldTime > 0.0)
	{
		double CurrentWorldTime = FPlatformTime::Seconds();
		float ElapsedTime = static_cast<float>(CurrentWorldTime - MusicStartWorldTime);
		float CalculatedTime = LastSeekPosition + ElapsedTime;

		// 确保时间不会超出音乐长度
		return FMath::Clamp(CalculatedTime, 0.0f, CurrentMusicDuration);
	}

	// 降级到当前计算方法
	return CurrentDuration;
}
