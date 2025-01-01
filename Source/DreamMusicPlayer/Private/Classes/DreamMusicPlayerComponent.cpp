// Copyright © Dream Moon Studio . Dream Moon All rights reserved


#include "Classes/DreamMusicPlayerComponent.h"

#include "ConstantQNRT.h"
#include "DreamMusicPlayerBlueprint.h"
#include "Algo/RandomShuffle.h"
#include "Containers/Array.h"
#include "DreamMusicPlayerLog.h"
#include "LoudnessNRT.h"
#include "Classes/DreamMusicData.h"
#include "Classes/DreamMusicPlayerLyricTools.h"
#include "Kismet/KismetMathLibrary.h"

void UDreamMusicPlayerComponent::BeginPlay()
{
	SubAudioComponentA = NewObject<UAudioComponent>(GetOwner(), TEXT("MusicPlayerAudioComponentA"));
	SubAudioComponentA->SetupAttachment(GetOwner()->GetRootComponent());
	SubAudioComponentA->RegisterComponent();

	SubAudioComponentB = NewObject<UAudioComponent>(GetOwner(), TEXT("MusicPlayerAudioComponentB"));
	SubAudioComponentB->SetupAttachment(GetOwner()->GetRootComponent());
	SubAudioComponentB->RegisterComponent();

	if (SongList)
	{
		InitializeMusicList();
	}

	Super::BeginPlay();
}

void UDreamMusicPlayerComponent::SetPlayMode(EDreamMusicPlayerPlayMode InPlayMode)
{
	PlayMode = InPlayMode;
	OnPlayModeChanged.Broadcast(PlayMode);
}

void UDreamMusicPlayerComponent::InitializeLyricList()
{
	if (!CurrentMusicData.IsVaild())
	{
		DMP_LOG(Error, TEXT("CurrentMusicData is Not Valid"));
		return;
	}
	DMP_LOG(Log, TEXT("InitializeLyricList - Begin"));
	CurrentMusicLyricList.Empty();
	CurrentMusicLyricList = FDreamMusicPlayerLyricTools::LoadLyricFromFile(
		FDreamMusicPlayerLyricTools::GetLyricFilePath(CurrentMusicData.Data.LyricFileName));
	OnLyricListChanged.Broadcast(CurrentMusicLyricList);
	DMP_LOG(Log, TEXT("InitializeLyricList Count : %02d - End"), CurrentMusicLyricList.Num());
}

void UDreamMusicPlayerComponent::InitializeMusicList()
{
	DMP_LOG(Log, TEXT("InitializeMusicList - Begin"));
	TArray<FDreamMusicPlayerSondList*> BufferList;
	MusicDataList.Empty();
	SongList->GetAllRows<FDreamMusicPlayerSondList>("", BufferList);
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

void UDreamMusicPlayerComponent::PlayMusicWithLyric(FDreamMusicLyric InLyric)
{
	if (CurrentMusicLyricList.Contains(InLyric))
	{
		float Time = FDreamMusicPlayerLyricTools::Conv_FloatFromTimestamp(InLyric.Timestamp);
		Time = UKismetMathLibrary::NormalizeToRange(Time, 0.0f, CurrentMusicDuration);
		SetMusicPercent(Time);
	}
	else
	{
		DMP_LOG(Warning, TEXT("%hs Lyric Not Found !!!"), __FUNCTION__)
	}
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
		return CurrentMusicData.IsVaild() ? CurrentMusicData : InData;
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
		return CurrentMusicData.IsVaild() ? CurrentMusicData : InData;
	}
	return MusicDataList[(MusicDataList.Find(InData) - 1 < 0)
		                     ? MusicDataList.Num() - 1
		                     : MusicDataList.Find(InData) - 1];
}

void UDreamMusicPlayerComponent::GetAudioNrtData(bool bConstantReverse, TArray<float>& ConstantNrtL, TArray<float>& ConstantNrtR,
                                                 TArray<float>& ConstantNrtAverage,
                                                 float& OutLoudnessValue)
{
	ConstantNrtL = ConstantQDataL;
	ConstantNrtR = ConstantQDataR;
	TArray<float> ConstantQData, ConstantQDataAverage;
	if (bConstantReverse)
	{
		ConstantQData.Append(ConstantQDataR);
		ConstantQData.Append(ConstantQDataL);
	}
	else
	{
		ConstantQData.Append(ConstantQDataL);
		ConstantQData.Append(ConstantQDataR);
	}	
	
	for (int i = 0; i < ConstantQData.Num() / 2; ++i)
	{
		ConstantQDataAverage.Add((ConstantQData[i] + ConstantQData[ConstantQData.Num() - i - 1]) / 2);
	}
	ConstantNrtAverage = ConstantQDataAverage;
	OutLoudnessValue = LoudnessValue;
}

UAudioComponent* UDreamMusicPlayerComponent::GetActiveAudioComponent() const
{
	return CurrentActiveAudioComponent ? SubAudioComponentB : SubAudioComponentA;
}

UAudioComponent* UDreamMusicPlayerComponent::GetLastActiveAudioComponent() const
{
	return CurrentActiveAudioComponent ? SubAudioComponentA : SubAudioComponentB;
}

TArray<FString> UDreamMusicPlayerComponent::GetNames() const
{
	return UDreamMusicPlayerBlueprint::GetLyricFileNames();
}

void UDreamMusicPlayerComponent::StartMusic()
{
	if (!CurrentMusicData.IsVaild())
	{
		DMP_LOG(Error, TEXT("Current Music Data Is Not Valid !!!"))
		return;
	}

	// Initialize State
	CurrentMusicDuration = 0.0f;
	CurrentMusicPercent = 0.0f;
	CurrentDuration = 0.0f;
	CurrentTimestamp = FDreamMusicLyricTimestamp();
	ToggleActiveAudioComponent();

	InitializeLyricList();

	// Play Music
	CurrentMusicDuration = SoundWave->Duration;
	GetActiveAudioComponent()->SetSound(SoundWave);
	GetActiveAudioComponent()->Play();

	// Fade In
	GetActiveAudioComponent()->FadeIn(FadeAudioSetting.bEnableFadeAudio ? FadeAudioSetting.FadeInDuration : 0.0f);

	bIsPaused = false;
	bIsPlaying = true;
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Playing);

	// Clean Timer And Start Timer
	GetOwner()->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	GetOwner()->GetWorld()->GetTimerManager().SetTimer(
		TimerHandle, this, &UDreamMusicPlayerComponent::MusicTick,
		UpdateTime, true
	);

	// Callback
	OnMusicPlay.Broadcast(CurrentMusicData);
	DMP_LOG(Log, TEXT("Play Music : Name : %-15s Duration : %f"), *CurrentMusicData.Information.Title,
	        CurrentMusicDuration)
}

void UDreamMusicPlayerComponent::EndMusic(bool Native)
{
	// Stop Music
	GetActiveAudioComponent()->FadeOut(FadeAudioSetting.bEnableFadeAudio ? FadeAudioSetting.FadeOutDuration : 0.0f,
	                                   0.0f);
	if (GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
		GetActiveAudioComponent()->Stop();
	}
	else
	{
		GWorld->GetTimerManager().SetTimer(StopTimerHandle, GetActiveAudioComponent(), &UAudioComponent::Stop,
									   FadeAudioSetting.bEnableFadeAudio ? FadeAudioSetting.FadeOutDuration : 0.0f);
	}
	bIsPaused = false;
	bIsPlaying = false;
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Stop);

	// Clear Timer
	GetOwner()->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	// Free Memory
	CurrentDuration = 0.0f;
	CurrentMusicDuration = 0.0f;
	CurrentMusicPercent = 0.0f;
	ConstantQDataL.Empty();
	ConstantQDataR.Empty();
	LoudnessValue = 0.0f;
	ConstantQ = nullptr;
	Loudness = nullptr;
	OnMusicEnd.Broadcast();
	DMP_LOG(Log, TEXT("Music End : Name : %-15s Play Mode : %d"), *CurrentMusicData.Information.Title, (int)PlayMode);
	DMP_LOG(Log, TEXT("Music End : Name : %-15s Play Mode : %d"), *CurrentMusicData.Information.Title, (int)PlayMode);

	if (!Native)
	{
		switch (PlayMode)
		{
		case EDreamMusicPlayerPlayMode::EDMPPS_Loop:
			SetMusicData(CurrentMusicData);
			StartMusic();
			break;
		case EDreamMusicPlayerPlayMode::EDMPPS_Normal:
			PlayNextMusic();
			break;
		case EDreamMusicPlayerPlayMode::EDMPPS_Random:
			PlayNextMusic();
			break;
		}
	}
}

void UDreamMusicPlayerComponent::PauseMusic()
{
	GetActiveAudioComponent()->SetPaused(true);
	GetOwner()->GetWorld()->GetTimerManager().PauseTimer(TimerHandle);
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Paused);
	bIsPaused = true;
	OnMusicPause.Broadcast();
}

void UDreamMusicPlayerComponent::UnPauseMusic()
{
	GetActiveAudioComponent()->SetPaused(false);
	GetOwner()->GetWorld()->GetTimerManager().UnPauseTimer(TimerHandle);
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Playing);
	bIsPaused = false;
	OnMusicUnPause.Broadcast();
}

void UDreamMusicPlayerComponent::SetMusicData(FDreamMusicDataStruct InData)
{
	CurrentMusicData = InData;
	
	LoadAudioNrt();
	SoundWave = CurrentMusicData.Data.Music.LoadSynchronous();
	Cover = CurrentMusicData.Information.Cover.LoadSynchronous();
	
	OnMusicDataChanged.Broadcast(CurrentMusicData);
}

void UDreamMusicPlayerComponent::SetPlayState(EDreamMusicPlayerPlayState InState)
{
	PlayState = InState;
	OnPlayStateChanged.Broadcast(PlayState);
}

void UDreamMusicPlayerComponent::SetMusicPercent(float InPercent)
{
	InPercent = FMath::Clamp(InPercent, 0.0f, 1.0f);
	CurrentMusicPercent = InPercent;
	CurrentDuration = CurrentMusicDuration * InPercent;
	CurrentTimestamp = FDreamMusicPlayerLyricTools::Conv_TimestampFromFloat(
		CurrentDuration + LyricOffset);
	GetActiveAudioComponent()->Play(CurrentDuration);
	DMP_LOG(Log, TEXT("Set Music Percent : %f Time : %02d:%02d.%02d"), CurrentMusicPercent, CurrentTimestamp.Minute,
	        CurrentTimestamp.Seconds, CurrentTimestamp.Millisecond);
}

void UDreamMusicPlayerComponent::SetMusicPercentFromTimestamp(FDreamMusicLyricTimestamp InTimestamp)
{
	SetMusicPercent(FDreamMusicPlayerLyricTools::Conv_FloatFromTimestamp(InTimestamp) / CurrentMusicDuration);
}


void UDreamMusicPlayerComponent::SetCurrentLyric(FDreamMusicLyric InLyric)
{
	if (InLyric != CurrentLyric && InLyric.IsNotEmpty())
	{
		CurrentLyric = InLyric;
		OnLyricChanged.Broadcast(CurrentLyric, CurrentMusicLyricList.Find(CurrentLyric));
		DMP_LOG(Log, TEXT("Lyric : Set : Time : %02d-%02d.%02d Content : %s"),
		        InLyric.Timestamp.Minute, InLyric.Timestamp.Seconds, InLyric.Timestamp.Millisecond, *InLyric.Content);
	}
}

void UDreamMusicPlayerComponent::LoadAudioNrt()
{
	if (CurrentMusicData.Data.ConstantQ.IsValid())
		ConstantQ = Cast<UConstantQNRT>(CurrentMusicData.Data.ConstantQ.TryLoad());
	if (CurrentMusicData.Data.Loudness.IsValid())
		Loudness = Cast<ULoudnessNRT>(CurrentMusicData.Data.Loudness.TryLoad());
	DMP_LOG(Log, TEXT("Load Audio NRT Done."))
}

void UDreamMusicPlayerComponent::MusicTick()
{
	// Check Music Is Ended

	if (CurrentDuration >= CurrentMusicDuration - (FadeAudioSetting.bEnableFadeAudio
		                                               ? FadeAudioSetting.FadeOutDuration
		                                               : 0.0f))
	{
		DMP_LOG(Log, TEXT("Music Tick Music Name : %s - End"), *CurrentMusicData.Information.Title)
		EndMusic();

		return;
	}

	// Begin Music Tick

	CurrentDuration += UpdateTime;
	CurrentMusicPercent = CurrentDuration / CurrentMusicDuration;
	CurrentTimestamp = FDreamMusicPlayerLyricTools::Conv_TimestampFromFloat(
		CurrentDuration + LyricOffset);
	SetCurrentLyric(FDreamMusicPlayerLyricTools::GetCurrentLyric(CurrentTimestamp, CurrentMusicLyricList));
	if (ConstantQ)
	{
		ConstantQ.Get()->GetNormalizedChannelConstantQAtTime(CurrentDuration, 0, ConstantQDataL);
		ConstantQ.Get()->GetNormalizedChannelConstantQAtTime(CurrentDuration, 1, ConstantQDataR);
	}
	if (Loudness)
	{
		Loudness.Get()->GetNormalizedLoudnessAtTime(CurrentDuration, LoudnessValue);
	}
	OnMusicTick.Broadcast(CurrentDuration);

	// DMP_LOG(Log, TEXT("Music Tick - Time : %02d-%02d.%02d"), Timestamp.Minute, Timestamp.Seconds, Timestamp.Millisecond)
}

bool UDreamMusicPlayerComponent::ToggleActiveAudioComponent()
{
	CurrentActiveAudioComponent = !CurrentActiveAudioComponent;
	DMP_LOG(Log, TEXT("Toggle Active Audio Component : %d"), CurrentActiveAudioComponent)
	return CurrentActiveAudioComponent;
}
