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
	// Create audio components with better configuration
	SubAudioComponentA = NewObject<UAudioComponent>(GetOwner(), TEXT("MusicPlayerAudioComponentA"));
	if (SubAudioComponentA)
	{
		SubAudioComponentA->SetupAttachment(GetOwner()->GetRootComponent());
		SubAudioComponentA->bAutoActivate = false; // Prevent auto-activation
		if (SoundClass)
		{
			SubAudioComponentA->SoundClassOverride = SoundClass;
		}
		SubAudioComponentA->RegisterComponent();
	}

	SubAudioComponentB = NewObject<UAudioComponent>(GetOwner(), TEXT("MusicPlayerAudioComponentB"));
	if (SubAudioComponentB)
	{
		SubAudioComponentB->SetupAttachment(GetOwner()->GetRootComponent());
		SubAudioComponentB->bAutoActivate = false; // Prevent auto-activation
		if (SoundClass)
		{
			SubAudioComponentB->SoundClassOverride = SoundClass;
		}
		SubAudioComponentB->RegisterComponent();
	}

	if (SongList)
	{
		InitializeMusicList();
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

	// Clean up timers
	if (GWorld && GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	// Cancel any pending async operations
	if (CurrentKMeansTask && CurrentKMeansTask->IsValidLowLevel())
	{
		CurrentKMeansTask->Cancel();
		CurrentKMeansTask = nullptr;
	}

	// Clean up audio components
	if (SubAudioComponentA && SubAudioComponentA->IsValidLowLevel())
	{
		SubAudioComponentA->Stop();
	}
	if (SubAudioComponentB && SubAudioComponentB->IsValidLowLevel())
	{
		SubAudioComponentB->Stop();
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

void UDreamMusicPlayerComponent::PlayMusicWithLyric(FDreamMusicLyric InLyric)
{
	if (CurrentMusicLyricList.Contains(InLyric))
	{
		float Time = InLyric.StartTimestamp.ToSeconds();
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
	UAudioComponent* Component = CurrentActiveAudioComponent ? SubAudioComponentB : SubAudioComponentA;
	return IsAudioComponentReady(Component) ? Component : nullptr;
}


UAudioComponent* UDreamMusicPlayerComponent::GetLastActiveAudioComponent() const
{
	UAudioComponent* Component = CurrentActiveAudioComponent ? SubAudioComponentA : SubAudioComponentB;
	return IsAudioComponentReady(Component) ? Component : nullptr;
}

FDreamMusicLyricProgress UDreamMusicPlayerComponent::GetCurrentLyricWordProgress(const FDreamMusicLyricTimestamp& InTimestamp) const
{
	return CalculateWordProgress(InTimestamp);
}

FDreamMusicLyricProgress UDreamMusicPlayerComponent::GetCurrentRomanizationProgress(const FDreamMusicLyricTimestamp& InTimestamp) const
{
	return CalculateWordProgress(InTimestamp, true);
}

FDreamMusicLyricProgress UDreamMusicPlayerComponent::GetCurrentLyricLineProgress(const FDreamMusicLyricTimestamp& InTimestamp) const
{
	return CalculateLineProgress(InTimestamp);
}

void UDreamMusicPlayerComponent::ExtractCoverThemeColors(int32 ClusterCount, int32 MaxIterations)
{
	if (!Cover || !Cover->IsValidLowLevel())
	{
		DMP_LOG(Warning, TEXT("No valid cover image available for theme color extraction"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// Clean up existing task more safely
	if (CurrentKMeansTask)
	{
		if (CurrentKMeansTask->IsValidLowLevel() && !CurrentKMeansTask->IsCompletedOrCancelled())
		{
			DMP_LOG(Log, TEXT("Cancelling previous K-Means task before starting new one"));
			CurrentKMeansTask->Cancel();
		}
		CurrentKMeansTask = nullptr;
	}

	ExtractTextureThemeColors(Cover, ClusterCount, MaxIterations);
}


void UDreamMusicPlayerComponent::ExtractTextureThemeColors(UTexture2D* Texture, int32 ClusterCount, int32 MaxIterations)
{
	if (!Texture || !Texture->IsValidLowLevel())
	{
		DMP_LOG(Warning, TEXT("Invalid texture for theme color extraction - Texture is null or invalid"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// Enhanced texture validation
	const FTexturePlatformData* PlatformData = Texture->GetPlatformData();
	if (!PlatformData || PlatformData->Mips.Num() == 0)
	{
		DMP_LOG(Warning, TEXT("Invalid texture for theme color extraction - No platform data or mips"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	const FTexture2DMipMap& MipMap = PlatformData->Mips[0];
	if (MipMap.BulkData.GetBulkDataSize() == 0)
	{
		DMP_LOG(Warning, TEXT("Invalid texture for theme color extraction - Empty bulk data"));
		OnThemeColorChanged.Broadcast(TArray<FKMeansColorCluster>(), false);
		return;
	}

	// Clean up existing task safely
	if (CurrentKMeansTask)
	{
		if (CurrentKMeansTask->IsValidLowLevel() && !CurrentKMeansTask->IsCompletedOrCancelled())
		{
			DMP_LOG(Log, TEXT("Cancelling existing K-Means task"));
			CurrentKMeansTask->Cancel();
		}
		CurrentKMeansTask = nullptr;
	}

	// Create new task with clamped parameters
	CurrentKMeansTask = UDreamAsyncAction_KMeansTexture::KMeansTextureAnalysis(
		Texture,
		FMath::Clamp(ClusterCount, 1, CoverThemeColorCount),
		FMath::Clamp(MaxIterations, 1, MaxIterationsCount),
		FMath::Clamp(SampleRate, 0.1f, 1.0f),
		bIgnoreTransparent,
		FMath::Clamp(AlphaThreshold, 0.0f, 1.0f)
	);

	if (CurrentKMeansTask && CurrentKMeansTask->IsValidLowLevel())
	{
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

bool UDreamMusicPlayerComponent::IsAudioComponentReady(UAudioComponent* Component) const
{
	return Component &&
		Component->IsValidLowLevel() &&
		Component->IsRegistered() &&
		!Component->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed);
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

void UDreamMusicPlayerComponent::UpdateAudioAnalysisData()
{
	if (ConstantQ && ConstantQ->IsValidLowLevel())
	{
		ConstantQ->GetNormalizedChannelConstantQAtTime(CurrentDuration, 0, ConstantQDataL);
		ConstantQ->GetNormalizedChannelConstantQAtTime(CurrentDuration, 1, ConstantQDataR);
	}

	if (Loudness && Loudness->IsValidLowLevel())
	{
		Loudness->GetNormalizedLoudnessAtTime(CurrentDuration, LoudnessValue);
	}
}

void UDreamMusicPlayerComponent::BuildWordDurationCache(bool bUseRoma) const
{
	if (bCacheValid && bLastUseRoma == bUseRoma)
	{
		return;
	}

	const TArray<FDreamMusicLyricWord>& Words = bUseRoma ? CurrentLyric.RomanizationWordTimings : CurrentLyric.WordTimings;
        
	WordDurationPrefixSum.Empty();
	WordDurationPrefixSum.Reserve(Words.Num());
        
	int32 CumulativeDuration = 0;
	for (const FDreamMusicLyricWord& Word : Words)
	{
		int32 WordDuration = Word.EndTimestamp.ToMilliseconds() - Word.StartTimestamp.ToMilliseconds();
		CumulativeDuration += WordDuration;
		WordDurationPrefixSum.Add(CumulativeDuration);
	}
        
	bCacheValid = true;
	bLastUseRoma = bUseRoma;
}

void UDreamMusicPlayerComponent::OnThemeColorsExtracted(const TArray<FKMeansColorCluster>& ColorClusters, bool bSuccess)
{
	// Safely clear the current task reference
	UDreamAsyncAction_KMeansTexture* CompletedTask = CurrentKMeansTask;
	CurrentKMeansTask = nullptr;

	if (bSuccess && ColorClusters.Num() > 0)
	{
		DMP_LOG(Log, TEXT("Theme color extraction completed successfully. Found %d colors"), ColorClusters.Num());

		// Log extracted colors for debugging
		for (int32 i = 0; i < ColorClusters.Num(); ++i)
		{
			const FKMeansColorCluster& Cluster = ColorClusters[i];
			DMP_LOG(Log, TEXT("Color %d: R=%.3f G=%.3f B=%.3f Weight=%.3f PixelCount=%d"),
			        i, Cluster.Color.R, Cluster.Color.G, Cluster.Color.B, Cluster.Weight, Cluster.PixelCount);
		}
	}
	else
	{
		// Enhanced error reporting
		if (!bSuccess)
		{
			DMP_LOG(Warning, TEXT("Theme color extraction failed or was cancelled"));
		}
		else
		{
			DMP_LOG(Warning, TEXT("Theme color extraction found no colors"));
		}

		// Additional debugging info if task is available
		if (CompletedTask && CompletedTask->IsValidLowLevel())
		{
			DMP_LOG(Warning, TEXT("Task details - Texture: %s, SampledPixels: %d"),
			        CompletedTask->GetTargetTexture() ? *CompletedTask->GetTargetTexture()->GetName() : TEXT("NULL"),
			        CompletedTask->GetSamplePixelNum());
		}
	}

	OnThemeColorChanged.Broadcast(ColorClusters, bSuccess);
}

FDreamMusicLyricProgress UDreamMusicPlayerComponent::CalculateWordProgress(FDreamMusicLyricTimestamp InCurrentTime, bool bUseRoma) const
{
	// 边界检查
	if (CurrentLyric.IsEmpty())
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		return FDreamMusicLyricProgress(0, 0.0f, false, FDreamMusicLyricWord{});
	}

	// 如果当前时间在歌词行开始之前，返回0
	if (InCurrentTime < CurrentLyric.StartTimestamp)
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		return FDreamMusicLyricProgress(0, 0.0f, false, FDreamMusicLyricWord{});
	}

	// 如果当前时间在歌词行结束之后，返回1（完成状态）
	if (InCurrentTime > CurrentLyric.EndTimestamp)
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		return FDreamMusicLyricProgress(-1, 1.0f, false, FDreamMusicLyricWord{});
	}

	// 如果没有单词时间信息，使用行进度
	if (bUseRoma ? CurrentLyric.IsRomanizationWordsEmpty() : CurrentLyric.IsWordsEmpty())
	{
		return CalculateLineProgress(InCurrentTime);
	}

	// 构建缓存
	BuildWordDurationCache(bUseRoma);

	const TArray<FDreamMusicLyricWord>& Words = bUseRoma ? CurrentLyric.RomanizationWordTimings : CurrentLyric.WordTimings;

	// 性能优化：从上次位置开始查找，通常时间是递增的
	int32 StartIndex = 0;
	if (CachedCurrentWordIndex >= 0 && CachedCurrentWordIndex < Words.Num() &&
		InCurrentTime >= LastCalculationTime)
	{
		StartIndex = CachedCurrentWordIndex;
	}

	// 查找当前单词
	int32 CurrentWordIndex = -1;
	for (int32 i = StartIndex; i < Words.Num(); i++)
	{
		const FDreamMusicLyricWord& Word = Words[i];
		if (InCurrentTime >= Word.StartTimestamp && InCurrentTime < Word.EndTimestamp)
		{
			CurrentWordIndex = i;
			break;
		}
	}

	// 如果从缓存位置没找到，从头查找
	if (CurrentWordIndex == -1 && StartIndex > 0)
	{
		for (int32 i = 0; i < StartIndex; i++)
		{
			const FDreamMusicLyricWord& Word = Words[i];
			if (InCurrentTime >= Word.StartTimestamp && InCurrentTime < Word.EndTimestamp)
			{
				CurrentWordIndex = i;
				break;
			}
		}
	}

	// 更新缓存
	CachedCurrentWordIndex = CurrentWordIndex;
	LastCalculationTime = InCurrentTime;

	int32 LineTotalDuration = CurrentLyric.EndTimestamp.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();

	if (CurrentWordIndex >= 0)
	{
		const FDreamMusicLyricWord& CurrentWord = Words[CurrentWordIndex];

		// 使用前缀和快速计算进度
		int32 ProgressToWordStart = (CurrentWordIndex > 0) ? WordDurationPrefixSum[CurrentWordIndex - 1] : 0;
		int32 CurrentWordElapsed = InCurrentTime.ToMilliseconds() - CurrentWord.StartTimestamp.ToMilliseconds();
		int32 TotalProgress = ProgressToWordStart + CurrentWordElapsed;

		float LineProgress = static_cast<float>(TotalProgress) / static_cast<float>(LineTotalDuration);

		return FDreamMusicLyricProgress(CurrentWordIndex, LineProgress, true, CurrentWord);
	}
	else
	{
		// 回退到行进度计算
		int32 Elapsed = InCurrentTime.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();
		float LineProgress = static_cast<float>(Elapsed) / static_cast<float>(LineTotalDuration);

		return FDreamMusicLyricProgress(-1, LineProgress, false, FDreamMusicLyricWord{});
	}
}

FDreamMusicLyricProgress UDreamMusicPlayerComponent::CalculateLineProgress(FDreamMusicLyricTimestamp InCurrentTime) const
{
	if (InCurrentTime < CurrentLyric.StartTimestamp || InCurrentTime > CurrentLyric.EndTimestamp)
	{
		return FDreamMusicLyricProgress(-1, 0.0f, false, FDreamMusicLyricWord{});
	}

	int32 LineDuration = CurrentLyric.EndTimestamp.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();
	int32 Elapsed = InCurrentTime.ToMilliseconds() - CurrentLyric.StartTimestamp.ToMilliseconds();
    
	// 修复整数除法问题
	float Progress = static_cast<float>(Elapsed) / static_cast<float>(LineDuration);
    
	return FDreamMusicLyricProgress(-1, Progress, false, FDreamMusicLyricWord{});
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

	// Clear any existing timers to prevent conflicts
	if (GWorld && GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	// Initialize State
	CurrentMusicDuration = 0.0f;
	CurrentMusicPercent = 0.0f;
	CurrentDuration = 0.0f;
	LastSeekPosition = 0.0f;
	MusicStartWorldTime = FPlatformTime::Seconds(); // 记录开始时间
	bJustSeeked = false;
	CurrentTimestamp = FDreamMusicLyricTimestamp();

	// Switch audio components for seamless crossfading
	ToggleActiveAudioComponent();

	InitializeLyricList();

	// Validate SoundWave before playing
	if (!SoundWave || !SoundWave->IsValidLowLevel())
	{
		DMP_LOG(Error, TEXT("Invalid SoundWave for music: %s"), *CurrentMusicData.Information.Title);
		return;
	}

	// Set up audio component
	UAudioComponent* ActiveComponent = GetActiveAudioComponent();
	if (!ActiveComponent)
	{
		DMP_LOG(Error, TEXT("No valid audio component available"));
		return;
	}

	// Play Music with improved setup
	CurrentMusicDuration = SoundWave->Duration;
	ActiveComponent->SetSound(SoundWave);

	// Set volume to 0 before playing if fade-in is enabled
	if (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeInDuration > 0.0f)
	{
		ActiveComponent->SetVolumeMultiplier(0.0f);
	}

	ActiveComponent->Play();

	// Apply fade in
	if (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeInDuration > 0.0f)
	{
		ActiveComponent->FadeIn(FadeAudioSetting.FadeInDuration, 1.0f);
	}

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

	UAudioComponent* ActiveComponent = GetActiveAudioComponent();
	if (!ActiveComponent)
	{
		DMP_LOG(Warning, TEXT("No active audio component to stop"));
		return;
	}

	// Clear any existing stop timer
	if (GWorld && GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	// Calculate fade out duration
	float FadeOutDuration = (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeOutDuration > 0.0f)
		                        ? FadeAudioSetting.FadeOutDuration
		                        : 0.0f;

	// Start fade out
	if (FadeOutDuration > 0.0f)
	{
		ActiveComponent->FadeOut(FadeOutDuration, 0.0f);

		// Schedule stop after fade completes
		if (GWorld)
		{
			GWorld->GetTimerManager().SetTimer(
				StopTimerHandle,
				[this, ActiveComponent]()
				{
					if (ActiveComponent && ActiveComponent->IsValidLowLevel())
					{
						ActiveComponent->Stop();
					}
				},
				FadeOutDuration,
				false
			);
		}
	}
	else
	{
		// Stop immediately if no fade
		ActiveComponent->Stop();
	}

	// Update state immediately
	bIsPaused = false;
	bIsPlaying = false;
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Stop);

	// Clean up state
	CurrentDuration = 0.0f;
	CurrentMusicDuration = 0.0f;
	CurrentMusicPercent = 0.0f;
	ConstantQDataL.Empty();
	ConstantQDataR.Empty();
	LoudnessValue = 0.0f;

	// Safely clear references
	ConstantQ = nullptr;
	Loudness = nullptr;

	// Cancel any pending theme color extraction
	if (CurrentKMeansTask && CurrentKMeansTask->IsValidLowLevel())
	{
		CurrentKMeansTask->Cancel();
		CurrentKMeansTask = nullptr;
	}

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
			if (CurrentMusicData.IsVaild())
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
	GetActiveAudioComponent()->SetPaused(true);
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Paused);
	bIsPaused = true;

	// 暂停时保存当前精确时间，停止世界时间计算
	CurrentDuration = GetAccuratePlayTime();
	LastSeekPosition = CurrentDuration;
	MusicStartWorldTime = 0.0; // 停止世界时间基准

	OnMusicPause.Broadcast();
}

void UDreamMusicPlayerComponent::UnPauseMusic()
{
	GetActiveAudioComponent()->SetPaused(false);
	SetPlayState(EDreamMusicPlayerPlayState::EDMPPS_Playing);
	bIsPaused = false;

	// 恢复播放时重新设置时间基准
	MusicStartWorldTime = FPlatformTime::Seconds();
	bJustSeeked = true; // 标记为刚刚 Seek，使用保存的位置

	OnMusicUnPause.Broadcast();
}

void UDreamMusicPlayerComponent::SetMusicData(FDreamMusicDataStruct InData)
{
	CurrentMusicData = InData;

	LoadAudioNrt();
	SoundWave = CurrentMusicData.Data.Music.LoadSynchronous();
	Cover = CurrentMusicData.Information.Cover.LoadSynchronous();

	if (bUseThemeColorExtension)
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
	if (!bIsPlaying || !GetActiveAudioComponent())
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
	float LyricTime = CurrentDuration + LyricOffset;
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(LyricTime);

	// 停止并重新开始播放
	UAudioComponent* ActiveComponent = GetActiveAudioComponent();
	bool WasPlaying = ActiveComponent->IsPlaying();

	if (WasPlaying)
	{
		ActiveComponent->Stop();
	}

	// 从新位置开始播放
	ActiveComponent->Play(TargetTime);

	// 恢复暂停状态
	if (bIsPaused)
	{
		ActiveComponent->SetPaused(true);
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


void UDreamMusicPlayerComponent::SetCurrentLyric(FDreamMusicLyric InLyric)
{
	if (InLyric != CurrentLyric && InLyric.IsNotEmpty())
	{
		ClearLyricProgressCache();
		CurrentLyric = InLyric;
		OnLyricChanged.Broadcast(CurrentLyric, CurrentMusicLyricList.Find(CurrentLyric));
		DMP_LOG_DEBUG(Log, "Lyric", TEXT("Set : Time : %02d:%02d.%02d Content : %s"),
		              InLyric.StartTimestamp.Minute, InLyric.StartTimestamp.Seconds, InLyric.StartTimestamp.Millisecond, *InLyric.Content);
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
	// 获取更精确的播放时间
	float AccuratePlayTime = GetAccuratePlayTime();

	// 改进的音乐结束检测
	float EffectiveEndTime = CurrentMusicDuration;
	if (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeOutDuration > 0.0f)
	{
		EffectiveEndTime -= FadeAudioSetting.FadeOutDuration;
	}
	EffectiveEndTime -= 0.1f;

	if (AccuratePlayTime >= EffectiveEndTime)
	{
		DMP_LOG(Log, TEXT("Music Tick Music Name : %s - End"), *CurrentMusicData.Information.Title);
		EndMusic();
		return;
	}

	// 更新时间状态
	CurrentDuration = AccuratePlayTime;
	CurrentMusicPercent = FMath::Clamp(CurrentDuration / CurrentMusicDuration, 0.0f, 1.0f);

	// 应用歌词偏移
	float LyricTime = CurrentDuration + LyricOffset;
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(LyricTime);

	// 获取当前歌词（使用带偏移的时间）
	SetCurrentLyric(FDreamMusicPlayerLyricTools::GetLyricAtTimestamp(CurrentTimestamp, CurrentMusicLyricList));

	// 更新音频分析数据
	UpdateAudioAnalysisData();

	OnMusicTick.Broadcast(CurrentDuration);

	// 重置 Seek 标志
	if (bJustSeeked)
	{
		bJustSeeked = false;
	}
}

bool UDreamMusicPlayerComponent::ToggleActiveAudioComponent()
{
	CurrentActiveAudioComponent = !CurrentActiveAudioComponent;
	DMP_LOG(Log, TEXT("Toggle Active Audio Component : %d"), CurrentActiveAudioComponent)
	return CurrentActiveAudioComponent;
}
