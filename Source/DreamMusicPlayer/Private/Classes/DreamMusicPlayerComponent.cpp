// Copyright © Dream Moon Studio . Dream Moon All rights reserved


#include "Classes/DreamMusicPlayerComponent.h"

#include "Containers/Array.h"
#include "DreamMusicPlayerLog.h"
#include "Classes/DreamMusicDataAsset.h"
#include "AudioManager/DreamMusicAudioManager_Default.h"


UDreamMusicPlayerComponent::UDreamMusicPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	AudioManager = GetMutableDefault<UDreamMusicAudioManager_Default>();
}

void UDreamMusicPlayerComponent::BeginPlay()
{
	FApp::SetUnfocusedVolumeMultiplier(BackdropMusicVolume);

	if (SongList)
	{
		InitializeMusicList();
	}


	AudioManager->Initialize(this);
	for (int i = 0; i < ExpansionList.Num(); i++)
	{
		if (UDreamMusicPlayerExpansion* Expansion = ExpansionList[i])
		{
			Expansion->Initialize(this);
		}
		else
		{
			DMP_LOG(Error, TEXT("DreamMusicPlayerExpansion is null !!! index: %d"), i);
		}
	}

	Super::BeginPlay();
}

void UDreamMusicPlayerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	FApp::SetUnfocusedVolumeMultiplier(1.0f);

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
		if (Expansion == nullptr)
			continue;

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
	SongList->GetAllRows<FDreamMusicPlayerSongList>("", BufferList);

	MusicDataList.Empty(BufferList.Num());
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

void UDreamMusicPlayerComponent::InitializeMusicListWithDataArray(TArray<FDreamMusicData> InData)
{
	MusicDataList.Empty();
	MusicDataList = InData;
	OnMusicDataListChanged.Broadcast(MusicDataList);
}

void UDreamMusicPlayerComponent::PlayMusic(EDreamMusicPlayerPlayMode InPlayMode)
{
	PlayMode = InPlayMode;

	if (bIsPlaying)
	{
		EndMusic();
	}

	SetMusicData(MusicDataList[0], true);
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

	FDreamMusicData NextMusicData = GetNextMusicData(CurrentMusicData);

	if (PlayMode == EDreamMusicPlayerPlayMode::EDMPPS_Random)
	{
		NextMusicData = MusicDataList[FMath::RandRange(0, MusicDataList.Num() - 1)];
	}

	SetMusicData(NextMusicData, true);
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

	SetMusicData(GetLastMusicData(CurrentMusicData), true);
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

void UDreamMusicPlayerComponent::PlayMusicFromMusicData(FDreamMusicData InData)
{
	PlayMode = EDreamMusicPlayerPlayMode::EDMPPS_Loop;
	SetMusicData(InData, true);
}

void UDreamMusicPlayerComponent::PlayMusicFromMusicDataAsset(UDreamMusicDataAsset* InData)
{
	PlayMode = EDreamMusicPlayerPlayMode::EDMPPS_Loop;
	SetMusicData(InData->Data, true);
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
	CurrentTimestamp = *FDreamMusicTimestamp().FromSeconds(LyricTime);

	// 从新位置开始播放
	AudioManager->Music_SetPercent(TargetTime);

	for (UDreamMusicPlayerExpansion* Expansion : ExpansionList)
	{
		if (Expansion == nullptr)
			continue;

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

void UDreamMusicPlayerComponent::SetMusicPercentFromTimestamp(FDreamMusicTimestamp InTimestamp)
{
	SetMusicPercent(InTimestamp.ToSeconds() / CurrentMusicDuration);
}
