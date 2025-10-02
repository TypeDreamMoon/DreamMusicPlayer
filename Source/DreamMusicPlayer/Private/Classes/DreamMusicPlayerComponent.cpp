// Copyright © Dream Moon Studio . Dream Moon All rights reserved


#include "Classes/DreamMusicPlayerComponent.h"

#include "DreamMusicPlayerBlueprint.h"
#include "Algo/RandomShuffle.h"
#include "Containers/Array.h"
#include "DreamMusicPlayerLog.h"
#include "AudioManager/DreamMusicAudioManager_Default.h"
#include "LyricParser/DreamLyricParser.h"
#include "Classes/DreamMusicData.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "Classes/DreamMusicAudioManager.h"
#include "Classes/DreamMusicPlayerExpansionData.h"

UDreamMusicPlayerComponent::UDreamMusicPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	AudioManager = GetMutableDefault<UDreamMusicAudioManager_Default>();
}

void UDreamMusicPlayerComponent::BeginPlay()
{
	// Create audio components with better configuration

	if (SongList)
	{
		InitializeMusicList();
	}


	AudioManager->Initialize(this);
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->Initialize(this);
	}

	Super::BeginPlay();
}

void UDreamMusicPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop any playing music
	if (bIsPlaying)
	{
		EndMusic(true);
	}
	if (AudioManager)
	{
		AudioManager->Deinitialize();
	}
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->Deinitialize();
	}
	Super::EndPlay(EndPlayReason);
}

void UDreamMusicPlayerComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsPlaying && !bIsPaused)
	{
		MusicTick(DeltaTime);
	}
}

void UDreamMusicPlayerComponent::SetPlayMode(EDreamMusicPlayerPlayMode InPlayMode)
{
	PlayMode = InPlayMode;
	OnPlayModeChanged.Broadcast(PlayMode);
}


void UDreamMusicPlayerComponent::InitializeMusicList()
{
	DMP_LOG(Log, TEXT("InitializeMusicList - Begin"));
	TArray<FDreamMusicPlayerSongList*> BufferList;
	MusicDataList.Empty();
	SongList->GetAllRows<FDreamMusicPlayerSongList>("", BufferList);
	for (auto Element : BufferList)
	{
		if (Element)
		{
			MusicDataList.Add(Element->MusicData->Data);
		}
	}
	OnMusicDataListChanged.Broadcast(MusicDataList);
	DMP_LOG(Log, TEXT("InitializeMusicList Count : %02d - End"), MusicDataList.Num());
}

void UDreamMusicPlayerComponent::InitializeMusicListWithSongTable(UDataTable* Table)
{
	SongList = Table;
	InitializeMusicList();
}

void UDreamMusicPlayerComponent::InitializeMusicListWithDataArray(TArray<FDreamMusicDataStruct> InData)
{
	MusicDataList.Empty();
	MusicDataList = InData;
	OnMusicDataListChanged.Broadcast(MusicDataList);
}

void UDreamMusicPlayerComponent::PlayMusic(EDreamMusicPlayerPlayMode InPlayMode)
{
	PlayMode = InPlayMode;
	if (PlayMode == EDreamMusicPlayerPlayMode::EDMPPS_Random)
	{
		Algo::RandomShuffle(MusicDataList);
	}

	if (bIsPlaying)
	{
		EndMusic();
	}

	SetMusicData(MusicDataList[0]);
	StartMusic();
}

void UDreamMusicPlayerComponent::PlayNextMusic()
{
	if (MusicDataList.IsEmpty())
	{
		DMP_LOG(Warning, TEXT("Music List Is Empty !!!"));
		return;
	}

	if (bIsPlaying)
	{
		EndMusic(true);
	}

	SetMusicData(GetNextMusicData(CurrentMusicData));
	StartMusic();
}

void UDreamMusicPlayerComponent::PlayLastMusic()
{
	if (MusicDataList.IsEmpty())
	{
		DMP_LOG(Warning, TEXT("Music List Is Empty !!!"));
		return;
	}

	if (bIsPlaying)
	{
		EndMusic(true);
	}

	SetMusicData(GetLastMusicData(CurrentMusicData));
	StartMusic();
}

void UDreamMusicPlayerComponent::SetPauseMusic(bool bInPause)
{
	if (bInPause)
	{
		PauseMusic();
	}
	else
	{
		UnPauseMusic();
	}
}

void UDreamMusicPlayerComponent::TogglePauseMusic()
{
	if (bIsPaused)
	{
		UnPauseMusic();
	}
	else
	{
		PauseMusic();
	}
}

void UDreamMusicPlayerComponent::PlayMusicFromMusicData(FDreamMusicDataStruct InData)
{
	PlayMode = EDreamMusicPlayerPlayMode::EDMPPS_Loop;
	SetMusicData(InData);
	StartMusic();
}

void UDreamMusicPlayerComponent::PlayMusicFromMusicDataAsset(UDreamMusicData* InData)
{
	PlayMode = EDreamMusicPlayerPlayMode::EDMPPS_Loop;
	SetMusicData(InData->Data);
	StartMusic();
}

FDreamMusicDataStruct UDreamMusicPlayerComponent::GetNextMusicData(FDreamMusicDataStruct InData)
{
	if (PlayMode == EDreamMusicPlayerPlayMode::EDMPPS_Loop)
	{
		return CurrentMusicData.IsValid() ? CurrentMusicData : InData;
	}
	if (MusicDataList.Contains(InData))
	{
		return MusicDataList[(MusicDataList.Find(InData) + 1) > MusicDataList.Num() - 1
			                     ? 0
			                     : MusicDataList.Find(InData) + 1];
	}
	else
	{
		return MusicDataList[0];
	}
}

FDreamMusicDataStruct UDreamMusicPlayerComponent::GetLastMusicData(FDreamMusicDataStruct InData)
{
	if (PlayMode == EDreamMusicPlayerPlayMode::EDMPPS_Loop)
	{
		return CurrentMusicData.IsValid() ? CurrentMusicData : InData;
	}
	return MusicDataList[(MusicDataList.Find(InData) - 1 < 0)
		                     ? MusicDataList.Num() - 1
		                     : MusicDataList.Find(InData) - 1];
}

void UDreamMusicPlayerComponent::GetExpansionByClass(TSubclassOf<UDreamMusicPlayerExpansion> InExpansionClass, UDreamMusicPlayerExpansion*& OutExpansion) const
{
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion->GetClass() == InExpansionClass)
		{
			OutExpansion = Expansion;
			return;
		}
	}
}

bool UDreamMusicPlayerComponent::HasExpansion(TSubclassOf<UDreamMusicPlayerExpansion> InExpansionClass) const
{
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion->GetClass() == InExpansionClass)
		{
			return true;
		}
	}

	return false;
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

TArray<FString> UDreamMusicPlayerComponent::GetNames() const
{
	return UDreamMusicPlayerBlueprint::GetLyricFileNames();
}

void UDreamMusicPlayerComponent::StartMusic()
{
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
	CurrentTimestamp = FDreamMusicLyricTimestamp();

	// Validate SoundWave before playing
	if (!SoundWave || !SoundWave->IsValidLowLevel())
	{
		DMP_LOG(Error, TEXT("Invalid SoundWave for music: %s"), *CurrentMusicData.Information.Title);
		return;
	}

	// Play Music with improved setup
	CurrentMusicDuration = SoundWave->Duration;

	AudioManager->Music_Start();
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->MusicStart();
	}

	AudioManager->Music_Play();

	// Update state
	bIsPaused = false;
	bIsPlaying = true;
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Playing);

	// Callback
	OnMusicPlay.Broadcast(CurrentMusicData);
	DMP_LOG(Log, TEXT("Play Music : Name : %-15s Duration : %f"), *CurrentMusicData.Information.Title, CurrentMusicDuration);
}

void UDreamMusicPlayerComponent::EndMusic(bool Native)
{
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
	DMP_LOG(Log, TEXT("Music End : Name : %-15s Play Mode : %d"), *CurrentMusicData.Information.Title, (int)PlayMode);

	// Handle auto-play logic only if not manually stopped
	if (!Native)
	{
		// Add small delay to ensure clean transition
		if (GWorld)
		{
			GWorld->GetTimerManager().SetTimerForNextTick([this]()
			{
				HandleAutoPlayTransition();
			});
		}
	}
}

void UDreamMusicPlayerComponent::HandleAutoPlayTransition()
{
	if (!bIsPlaying) // Ensure we're still in stopped state
	{
		switch (PlayMode)
		{
		case EDreamMusicPlayerPlayMode::EDMPPS_Loop:
			if (CurrentMusicData.IsValid())
			{
				SetMusicData(CurrentMusicData);
				StartMusic();
			}
			break;
		case EDreamMusicPlayerPlayMode::EDMPPS_Normal:
		case EDreamMusicPlayerPlayMode::EDMPPS_Random:
			PlayNextMusic();
			break;
		}
	}
}

void UDreamMusicPlayerComponent::PauseMusic()
{
	AudioManager->Music_Pause();

	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Paused);
	bIsPaused = true;

	// 暂停时保存当前精确时间，停止世界时间计算
	CurrentDuration = GetAccuratePlayTime();
	LastSeekPosition = CurrentDuration;
	MusicStartWorldTime = 0.0; // 停止世界时间基准

	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->MusicPause();
	}

	OnMusicPause.Broadcast();
}

void UDreamMusicPlayerComponent::UnPauseMusic()
{
	AudioManager->Music_UnPause();

	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Playing);
	bIsPaused = false;

	// 恢复播放时重新设置时间基准
	MusicStartWorldTime = FPlatformTime::Seconds();
	bJustSeeked = true; // 标记为刚刚 Seek，使用保存的位置

	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->MusicUnPause();
	}

	OnMusicUnPause.Broadcast();
}

void UDreamMusicPlayerComponent::SetMusicData(FDreamMusicDataStruct InData)
{
	CurrentMusicData = InData;

	SoundWave = CurrentMusicData.Data.Music.LoadSynchronous();
	Cover = CurrentMusicData.Information.Cover.LoadSynchronous();

	AudioManager->Music_Changed(InData);
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->ChangeMusic(InData);
	}

	OnMusicDataChanged.Broadcast(CurrentMusicData);
}

void UDreamMusicPlayerComponent::SetPlayState(EDreamMusicPlayerPlayState InState)
{
	PlayState = InState;
	OnPlayStateChanged.Broadcast(PlayState);
}

void UDreamMusicPlayerComponent::SetMusicPercent(float InPercent)
{
	if (!bIsPlaying || !AudioManager->GetAudioComponent())
	{
		DMP_LOG(Warning, TEXT("Cannot set music percent when not playing or no active component"));
		return;
	}

	InPercent = FMath::Clamp(InPercent, 0.0f, 1.0f);
	CurrentMusicPercent = InPercent;

	// 计算目标时间
	float TargetTime = CurrentMusicDuration * InPercent;
	CurrentDuration = TargetTime;
	LastSeekPosition = TargetTime;
	bJustSeeked = true;

	// 重新设置开始时间基准
	MusicStartWorldTime = FPlatformTime::Seconds();

	// 应用歌词偏移
	float LyricTime = CurrentDuration;
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(LyricTime);

	// 停止并重新开始播放
	if (AudioManager->IsPlaying())
	{
		AudioManager->Music_Stop();
	}

	// 从新位置开始播放
	AudioManager->Music_Play(TargetTime);

	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->MusicSetPercent(InPercent);
	}

	// 恢复暂停状态
	if (bIsPaused)
	{
		AudioManager->Music_Pause();
		// 暂停时不更新世界时间基准
		MusicStartWorldTime = 0.0;
	}

	DMP_LOG(Log, TEXT("Set Music Percent: %.3f, Target Time: %.3f, Lyric Time: %.3f"),
	        CurrentMusicPercent, TargetTime, LyricTime);
}

void UDreamMusicPlayerComponent::SetMusicPercentFromTimestamp(FDreamMusicLyricTimestamp InTimestamp)
{
	SetMusicPercent(InTimestamp.ToSeconds() / CurrentMusicDuration);
}

void UDreamMusicPlayerComponent::MusicTick(float DeltaTime)
{
	float AccuratePlayTime = GetAccuratePlayTime();

	// 更新时间状态
	CurrentDuration = AccuratePlayTime;
	CurrentMusicPercent = FMath::Clamp(CurrentDuration / CurrentMusicDuration, 0.0f, 1.0f);
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(CurrentDuration);

	// Auto Next
	if (CurrentTimestamp >= CurrentMusicDuration)
	{
		EndMusic();
	}

	AudioManager->Tick(CurrentTimestamp, DeltaTime);
	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		Expansion->Tick(CurrentTimestamp, DeltaTime);
	}

	OnMusicTick.Broadcast(CurrentDuration);

	// 重置 Seek 标志
	if (bJustSeeked)
	{
		bJustSeeked = false;
	}
}
