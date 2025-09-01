// Copyright © Dream Moon Studio . Dream Moon All rights reserved


#include "Classes/DreamMusicPlayerComponent.h"

#include "ConstantQNRT.h"
#include "DreamMusicPlayerBlueprint.h"
#include "DreamMusicPlayerDebugLog.h"
#include "Algo/RandomShuffle.h"
#include "Containers/Array.h"
#include "DreamMusicPlayerLog.h"
#include "LoudnessNRT.h"
#include "Classes/DreamLyricParser.h"
#include "Classes/DreamMusicData.h"
#include "Classes/DreamMusicPlayerLyricTools.h"
#include "Kismet/KismetMathLibrary.h"
#include "AsyncAction/DreamAsyncAction_KMeansTexture.h"

UDreamMusicPlayerComponent::UDreamMusicPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

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

void UDreamMusicPlayerComponent::InitializeLyricList()
{
	if (!CurrentMusicData.IsVaild())
	{
		DMP_LOG(Error, TEXT("CurrentMusicData is Not Valid"));
		return;
	}
	DMP_LOG(Log, TEXT("InitializeLyricList - Begin"));
	CurrentMusicLyricList.Empty();

	FDreamLyricParser Parser(FDreamMusicPlayerLyricTools::GetLyricFilePath(CurrentMusicData.Data.LyricFileName),
	                         CurrentMusicData.Data.LyricParseFileType,
	                         CurrentMusicData.Data.LyricParseLineType,
	                         CurrentMusicData.Data.LrcLyricType);

	CurrentMusicLyricList = Parser.GetLyrics();

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
		float Time = InLyric.Timestamp.ToSeconds();
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

FDreamMusicLyricProgress UDreamMusicPlayerComponent::GetCurrentLyricWordProgress(float CurrentTime, const FDreamMusicLyric& InLyric) const
{
	if (InLyric.WordTimings.Num() > 0)
	{
		return CalculateWordProgress(CurrentTime, InLyric.WordTimings, InLyric.Timestamp.ToSeconds());
	}

	// Fallback to line-based progress if no word timings
	return GetCurrentLyricLineProgress(CurrentTime, InLyric);
}

FDreamMusicLyricProgress UDreamMusicPlayerComponent::GetCurrentRomanizationProgress(float CurrentTime, const FDreamMusicLyric& InLyric) const
{
	if (InLyric.RomanizationWordTimings.Num() > 0)
	{
		return CalculateWordProgress(CurrentTime, InLyric.RomanizationWordTimings, InLyric.Timestamp.ToSeconds());
	}

	// Fallback to line-based progress if no romanization word timings
	return GetCurrentLyricLineProgress(CurrentTime, InLyric);
}

FDreamMusicLyricProgress UDreamMusicPlayerComponent::GetCurrentLyricLineProgress(float CurrentTime, const FDreamMusicLyric& InLyric) const
{
	FDreamMusicLyricProgress Progress;

	float LineStartTime = InLyric.Timestamp.ToSeconds();
	float LineEndTime = InLyric.EndTimestamp.ToSeconds();

	// If EndTimestamp is not set or invalid, estimate duration
	if (LineEndTime <= LineStartTime)
	{
		// Estimate a reasonable duration (e.g., 3 seconds) or use next lyric timing
		LineEndTime = LineStartTime + 3.0f;
	}

	// Check if current time is within this lyric line
	if (CurrentTime >= LineStartTime && CurrentTime <= LineEndTime)
	{
		Progress.bIsActive = true;
		Progress.LineProgress = (CurrentTime - LineStartTime) / (LineEndTime - LineStartTime);
		Progress.LineProgress = FMath::Clamp(Progress.LineProgress, 0.0f, 1.0f);
		Progress.CurrentWordIndex = -1; // No specific word index for line-based progress
	}
	else if (CurrentTime < LineStartTime)
	{
		Progress.bIsActive = false;
		Progress.LineProgress = 0.0f;
		Progress.CurrentWordIndex = -1;
	}
	else
	{
		Progress.bIsActive = false;
		Progress.LineProgress = 1.0f;
		Progress.CurrentWordIndex = -1;
	}

	return Progress;
}

void UDreamMusicPlayerComponent::ExtractCoverThemeColors(int32 ClusterCount, int32 MaxIterations)
{
	if (!Cover)
	{
		DMP_LOG(Warning, TEXT("No cover image available for theme color extraction"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// 检查是否在快速切换歌曲时存在未完成的任务
	if (CurrentKMeansTask)
	{
		DMP_LOG(Warning, TEXT("Cancelling previous K-Means task before starting new one"));
		CurrentKMeansTask->Cancel();
		CurrentKMeansTask = nullptr;
	}

	ExtractTextureThemeColors(Cover, ClusterCount, MaxIterations);
}


void UDreamMusicPlayerComponent::ExtractTextureThemeColors(UTexture2D* Texture, int32 ClusterCount, int32 MaxIterations)
{
	if (!Texture)
	{
		DMP_LOG(Warning, TEXT("Invalid texture for theme color extraction - Texture is null"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// 添加更多验证
	if (!Texture->GetPlatformData())
	{
		DMP_LOG(Warning, TEXT("Invalid texture for theme color extraction - No platform data"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	if (Texture->GetPlatformData()->Mips.Num() == 0)
	{
		DMP_LOG(Warning, TEXT("Invalid texture for theme color extraction - No mip levels"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
	if (MipMap.BulkData.GetBulkDataSize() == 0)
	{
		DMP_LOG(Warning, TEXT("Invalid texture for theme color extraction - Empty bulk data"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// Cancel any existing task
	if (CurrentKMeansTask)
	{
		DMP_LOG(Log, TEXT("Cancelling existing K-Means task"));
		CurrentKMeansTask->Cancel();
		CurrentKMeansTask = nullptr;
	}

	// Start new K-Means analysis
	CurrentKMeansTask = UDreamAsyncAction_KMeansTexture::KMeansTextureAnalysis(
		Texture,
		FMath::Clamp(ClusterCount, 1, CoverThemeColorCount),
		FMath::Clamp(MaxIterations, 1, MaxIterationsCount),
		0.25f, // Sample 25% of pixels for performance
		true, // Ignore transparent pixels
		0.5f // Alpha threshold
	);

	if (CurrentKMeansTask)
	{
		// 添加任务开始前的调试信息
		DMP_LOG(Log, TEXT("Creating K-Means analysis task for texture: %s (Size: %dx%d, Format: %d)"),
		        *Texture->GetName(), Texture->GetSizeX(), Texture->GetSizeY(), (int)Texture->GetPixelFormat());

		CurrentKMeansTask->OnCompleted.AddDynamic(this, &UDreamMusicPlayerComponent::OnThemeColorsExtracted);
		CurrentKMeansTask->Activate();

		DMP_LOG(Log, TEXT("Started theme color extraction for texture: %s"), *Texture->GetName());
	}
	else
	{
		DMP_LOG(Warning, TEXT("Failed to create K-Means analysis task for texture: %s"), *Texture->GetName());
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
	}
}


void UDreamMusicPlayerComponent::OnThemeColorsExtracted(const TArray<FKMeansColorCluster>& ColorClusters, bool bSuccess)
{
	// 保存当前任务指针以便调试
	UDreamAsyncAction_KMeansTexture* CompletedTask = CurrentKMeansTask;
	CurrentKMeansTask = nullptr;

	if (bSuccess && ColorClusters.Num() > 0)
	{
		DMP_LOG(Log, TEXT("Theme color extraction completed successfully. Found %d colors"), ColorClusters.Num());

		// Log the extracted colors for debugging
		for (int32 i = 0; i < ColorClusters.Num(); ++i)
		{
			const FKMeansColorCluster& Cluster = ColorClusters[i];
			DMP_LOG(Log, TEXT("Color %d: R=%.3f G=%.3f B=%.3f Weight=%.3f PixelCount=%d"),
			        i, Cluster.Color.R, Cluster.Color.G, Cluster.Color.B, Cluster.Weight, Cluster.PixelCount);
		}
	}
	else
	{
		// 添加更详细的错误信息
		if (!bSuccess)
		{
			DMP_LOG(Warning, TEXT("Theme color extraction failed or was cancelled. Task may have been cancelled or encountered an error."));
		}
		else if (ColorClusters.Num() == 0)
		{
			DMP_LOG(Warning, TEXT("Theme color extraction completed but found no colors. This may indicate an issue with the texture or sampling."));
		}
		else
		{
			DMP_LOG(Warning, TEXT("Theme color extraction failed for unknown reasons."));
		}

		// 添加调试信息
		if (CompletedTask)
		{
			DMP_LOG(Warning, TEXT("Task details - IsCancelled: %s, SampledPixels: %d"),
			        CompletedTask->bIsCancelled ? TEXT("true") : TEXT("false"),
			        CompletedTask->GetSamplePixelNum());
		}
	}

	OnThemeColorChanged.Broadcast(ColorClusters, bSuccess);
}


FDreamMusicLyricProgress UDreamMusicPlayerComponent::CalculateWordProgress(float CurrentTime, const TArray<FDreamMusicLyricWord>& WordTimings, float LineStartTime) const
{
	FDreamMusicLyricProgress Progress;

	if (WordTimings.Num() == 0)
	{
		return Progress;
	}

	// WordTimings use absolute timestamps from song start, so use CurrentTime directly

	// Find the current word
	int32 CurrentWordIndex = -1;
	for (int32 i = 0; i < WordTimings.Num(); ++i)
	{
		float WordStartTime = WordTimings[i].StartTimestamp.ToSeconds();
		float WordEndTime = WordTimings[i].EndTimestamp.ToSeconds();

		// If EndTimestamp is not set, use start of next word or estimate
		if (WordEndTime <= WordStartTime)
		{
			if (i + 1 < WordTimings.Num())
			{
				WordEndTime = WordTimings[i + 1].StartTimestamp.ToSeconds();
			}
			else
			{
				WordEndTime = WordStartTime + 0.5f; // Estimate 0.5 seconds per word
			}
		}

		if (CurrentTime >= WordStartTime && CurrentTime < WordEndTime)
		{
			CurrentWordIndex = i;
			Progress.CurrentWord = WordTimings[i];
			break;
		}
	}

	Progress.CurrentWordIndex = CurrentWordIndex;

	// Calculate overall line progress based on word timings
	if (WordTimings.Num() > 0)
	{
		float LineStartTime_Local = WordTimings[0].StartTimestamp.ToSeconds();
		float LineEndTime_Local;

		// Get the end time of the last word
		if (WordTimings.Last().EndTimestamp.ToSeconds() > WordTimings.Last().StartTimestamp.ToSeconds())
		{
			LineEndTime_Local = WordTimings.Last().EndTimestamp.ToSeconds();
		}
		else
		{
			// Estimate end time if not provided
			LineEndTime_Local = WordTimings.Last().StartTimestamp.ToSeconds() + 0.5f;
		}

		if (CurrentTime >= LineStartTime_Local && CurrentTime <= LineEndTime_Local)
		{
			Progress.bIsActive = true;
			Progress.LineProgress = (CurrentTime - LineStartTime_Local) / (LineEndTime_Local - LineStartTime_Local);
			Progress.LineProgress = FMath::Clamp(Progress.LineProgress, 0.0f, 1.0f);
		}
		else if (CurrentTime < LineStartTime_Local)
		{
			Progress.bIsActive = false;
			Progress.LineProgress = 0.0f;
		}
		else
		{
			Progress.bIsActive = false;
			Progress.LineProgress = 1.0f;
		}
	}

	return Progress;
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
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Paused);
	bIsPaused = true;
	OnMusicPause.Broadcast();
}

void UDreamMusicPlayerComponent::UnPauseMusic()
{
	GetActiveAudioComponent()->SetPaused(false);
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

	ExtractCoverThemeColors(4, 10);

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
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(
		CurrentDuration + LyricOffset);
	GetActiveAudioComponent()->Play(CurrentDuration);
	DMP_LOG(Log, TEXT("Set Music Percent : %f Time : %02d:%02d.%02d"), CurrentMusicPercent, CurrentTimestamp.Minute,
	        CurrentTimestamp.Seconds, CurrentTimestamp.Millisecond);
}

void UDreamMusicPlayerComponent::SetMusicPercentFromTimestamp(FDreamMusicLyricTimestamp InTimestamp)
{
	SetMusicPercent(InTimestamp.ToSeconds() / CurrentMusicDuration);
}


void UDreamMusicPlayerComponent::SetCurrentLyric(FDreamMusicLyric InLyric)
{
	if (InLyric != CurrentLyric && InLyric.IsNotEmpty())
	{
		CurrentLyric = InLyric;
		OnLyricChanged.Broadcast(CurrentLyric, CurrentMusicLyricList.Find(CurrentLyric));
		DMP_LOG_DEBUG(Log, "Lyric", TEXT("Set : Time : %02d:%02d.%02d Content : %s"),
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

void UDreamMusicPlayerComponent::MusicTick(float DeltaTime)
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

	CurrentDuration += DeltaTime;
	CurrentMusicPercent = CurrentDuration / CurrentMusicDuration;
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(
		CurrentDuration + LyricOffset);
	SetCurrentLyric(FDreamMusicPlayerLyricTools::GetLyricAtTimestamp(CurrentTimestamp, CurrentMusicLyricList));
	if (ConstantQ)
	{
		ConstantQ.Get()->GetNormalizedChannelConstantQAtTime(CurrentDuration, 0, ConstantQDataL);
		ConstantQ.Get()->GetNormalizedChannelConstantQAtTime(CurrentDuration, 1, ConstantQDataR);
	}
	if (Loudness)
	{
		Loudness.Get()->GetNormalizedLoudnessAtTime(CurrentDuration, LoudnessValue);
	}

	if (!CurrentLyric.bIsEmptyLine)
	{
		FDreamMusicLyricProgress LyricProgress = GetCurrentLyricWordProgress(CurrentDuration, CurrentLyric);
		FDreamMusicLyricProgress RomanizationProgress = GetCurrentRomanizationProgress(CurrentDuration, CurrentLyric);

		// You can broadcast these or use them for UI updates
		// OnLyricProgressChanged.Broadcast(LyricProgress);
		// OnRomanizationProgressChanged.Broadcast(RomanizationProgress);

		DMP_LOG_DEBUG_TICK(Log, TEXT("Lyric Progress: %.2f, Word: %d, Romanization Progress: %.2f, Word: %d"),
		                   LyricProgress.LineProgress, LyricProgress.CurrentWordIndex,
		                   RomanizationProgress.LineProgress, RomanizationProgress.CurrentWordIndex);
	}

	OnMusicTick.Broadcast(CurrentDuration);

	DMP_LOG_DEBUG_TICK(Log, TEXT("Time: %s DeltaTime: %f P: %f"), *CurrentTimestamp.ToString(), DeltaTime, CurrentMusicPercent)
}

bool UDreamMusicPlayerComponent::ToggleActiveAudioComponent()
{
	CurrentActiveAudioComponent = !CurrentActiveAudioComponent;
	DMP_LOG(Log, TEXT("Toggle Active Audio Component : %d"), CurrentActiveAudioComponent)
	return CurrentActiveAudioComponent;
}
