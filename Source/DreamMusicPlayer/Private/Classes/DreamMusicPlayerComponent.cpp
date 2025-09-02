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
	UAudioComponent* Component = CurrentActiveAudioComponent ? SubAudioComponentB : SubAudioComponentA;
	return IsAudioComponentReady(Component) ? Component : nullptr;
}


UAudioComponent* UDreamMusicPlayerComponent::GetLastActiveAudioComponent() const
{
	UAudioComponent* Component = CurrentActiveAudioComponent ? SubAudioComponentA : SubAudioComponentB;
	return IsAudioComponentReady(Component) ? Component : nullptr;
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


FDreamMusicLyricProgress UDreamMusicPlayerComponent::CalculateWordProgress(float CurrentTime, const TArray<FDreamMusicLyricWord>& WordTimings, float LineStartTime) const
{
	FDreamMusicLyricProgress Progress;

	if (WordTimings.Num() == 0)
	{
		return Progress;
	}

	// Find the current word with improved timing logic
	int32 CurrentWordIndex = -1;
	float CurrentWordStartTime = 0.0f;
	float CurrentWordEndTime = 0.0f;

	for (int32 i = 0; i < WordTimings.Num(); ++i)
	{
		float WordStartTime = WordTimings[i].StartTimestamp.ToSeconds();
		float WordEndTime = WordTimings[i].EndTimestamp.ToSeconds();

		// Improved end time calculation
		if (WordEndTime <= WordStartTime)
		{
			if (i + 1 < WordTimings.Num())
			{
				// Use next word's start time
				WordEndTime = WordTimings[i + 1].StartTimestamp.ToSeconds();
			}
			else
			{
				// For the last word, use content-based estimation with minimum duration
				float ContentBasedDuration = FMath::Max(0.3f, WordTimings[i].Content.Len() * 0.08f + 0.2f);
				WordEndTime = WordStartTime + ContentBasedDuration;
			}
		}

		// Use slightly overlapping ranges to prevent gaps between words
		float WordEndTimeWithBuffer = (i == WordTimings.Num() - 1) ? WordEndTime + 0.15f : WordEndTime + 0.05f;

		if (CurrentTime >= WordStartTime && CurrentTime < WordEndTimeWithBuffer)
		{
			CurrentWordIndex = i;
			CurrentWordStartTime = WordStartTime;
			CurrentWordEndTime = WordEndTime;
			Progress.CurrentWord = WordTimings[i];
			break;
		}
	}

	Progress.CurrentWordIndex = CurrentWordIndex;

	// Calculate overall line progress with better end time handling
	float LineStartTime_Local = WordTimings[0].StartTimestamp.ToSeconds();
	float LineEndTime_Local;

	// Determine line end time more accurately
	const FDreamMusicLyricWord& LastWord = WordTimings.Last();
	if (LastWord.EndTimestamp.ToSeconds() > LastWord.StartTimestamp.ToSeconds())
	{
		LineEndTime_Local = LastWord.EndTimestamp.ToSeconds();
	}
	else
	{
		// Improved estimation for undefined end times
		float LastWordDuration = FMath::Max(0.5f, LastWord.Content.Len() * 0.08f + 0.3f);
		LineEndTime_Local = LastWord.StartTimestamp.ToSeconds() + LastWordDuration;
	}

	// Add adaptive buffer based on line content
	float AdaptiveBuffer = FMath::Clamp(WordTimings.Num() * 0.05f, 0.1f, 0.4f);
	LineEndTime_Local += AdaptiveBuffer;

	// Set line activity and progress
	if (CurrentTime >= LineStartTime_Local)
	{
		if (CurrentTime <= LineEndTime_Local)
		{
			Progress.bIsActive = true;
			float RawProgress = (CurrentTime - LineStartTime_Local) / (LineEndTime_Local - LineStartTime_Local);

			// Prevent premature completion of last word
			if (CurrentWordIndex == WordTimings.Num() - 1 && RawProgress > 0.9f)
			{
				// Ensure last word has enough time to complete
				float LastWordCompletionTime = CurrentWordEndTime + 0.1f;
				if (CurrentTime < LastWordCompletionTime)
				{
					RawProgress = FMath::Min(RawProgress, 0.9f);
				}
			}

			Progress.LineProgress = FMath::Clamp(RawProgress, 0.0f, 1.0f);
		}
		else
		{
			Progress.bIsActive = false;
			Progress.LineProgress = 1.0f;
		}
	}
	else
	{
		Progress.bIsActive = false;
		Progress.LineProgress = 0.0f;
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

	// Clear any existing timers to prevent conflicts
	if (GWorld && GWorld->GetTimerManager().TimerExists(StopTimerHandle))
	{
		GWorld->GetTimerManager().ClearTimer(StopTimerHandle);
	}

	// Initialize State
	CurrentMusicDuration = 0.0f;
	CurrentMusicPercent = 0.0f;
	CurrentDuration = 0.0f;
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
		? FadeAudioSetting.FadeOutDuration : 0.0f;

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
	CurrentDuration = CurrentMusicDuration * InPercent;
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(CurrentDuration + LyricOffset);
	
	// Stop current playback before seeking
	UAudioComponent* ActiveComponent = GetActiveAudioComponent();
	bool WasPlaying = ActiveComponent->IsPlaying();
	
	if (WasPlaying)
	{
		ActiveComponent->Stop();
	}
	
	// Seek to new position
	ActiveComponent->Play(CurrentDuration);
	
	// Restore pause state if needed
	if (bIsPaused)
	{
		ActiveComponent->SetPaused(true);
	}

	DMP_LOG(Log, TEXT("Set Music Percent : %f Time : %02d:%02d.%02d"), 
		CurrentMusicPercent, CurrentTimestamp.Minute, CurrentTimestamp.Seconds, CurrentTimestamp.Millisecond);
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
	// Improved end detection with fade consideration
	float EffectiveEndTime = CurrentMusicDuration;
	if (FadeAudioSetting.bEnableFadeAudio && FadeAudioSetting.FadeOutDuration > 0.0f)
	{
		EffectiveEndTime -= FadeAudioSetting.FadeOutDuration;
	}
	
	// Add small buffer to prevent premature ending
	EffectiveEndTime -= 0.1f;

	if (CurrentDuration >= EffectiveEndTime)
	{
		DMP_LOG(Log, TEXT("Music Tick Music Name : %s - End"), *CurrentMusicData.Information.Title);
		EndMusic();
		return;
	}

	// Update timing
	CurrentDuration += DeltaTime;
	CurrentMusicPercent = FMath::Clamp(CurrentDuration / CurrentMusicDuration, 0.0f, 1.0f);
	CurrentTimestamp = *FDreamMusicLyricTimestamp().FromSeconds(CurrentDuration + LyricOffset);
	
	// Update current lyric
	SetCurrentLyric(FDreamMusicPlayerLyricTools::GetLyricAtTimestamp(CurrentTimestamp, CurrentMusicLyricList));
	
	// Update audio analysis data safely
	if (ConstantQ && ConstantQ->IsValidLowLevel())
	{
		ConstantQ->GetNormalizedChannelConstantQAtTime(CurrentDuration, 0, ConstantQDataL);
		ConstantQ->GetNormalizedChannelConstantQAtTime(CurrentDuration, 1, ConstantQDataR);
	}
	
	if (Loudness && Loudness->IsValidLowLevel())
	{
		Loudness->GetNormalizedLoudnessAtTime(CurrentDuration, LoudnessValue);
	}

	// Handle lyric progress tracking
	if (!CurrentLyric.bIsEmptyLine && CurrentLyric.IsNotEmpty())
	{
		FDreamMusicLyricProgress LyricProgress = GetCurrentLyricWordProgress(CurrentDuration, CurrentLyric);
		FDreamMusicLyricProgress RomanizationProgress = GetCurrentRomanizationProgress(CurrentDuration, CurrentLyric);

		DMP_LOG_DEBUG_TICK(Log, TEXT("Lyric Progress: %.2f, Word: %d, Romanization Progress: %.2f, Word: %d"),
			LyricProgress.LineProgress, LyricProgress.CurrentWordIndex,
			RomanizationProgress.LineProgress, RomanizationProgress.CurrentWordIndex);
	}

	OnMusicTick.Broadcast(CurrentDuration);
	DMP_LOG_DEBUG_TICK(Log, TEXT("Time: %s DeltaTime: %f P: %f"), *CurrentTimestamp.ToString(), DeltaTime, CurrentMusicPercent);
}

bool UDreamMusicPlayerComponent::ToggleActiveAudioComponent()
{
	CurrentActiveAudioComponent = !CurrentActiveAudioComponent;
	DMP_LOG(Log, TEXT("Toggle Active Audio Component : %d"), CurrentActiveAudioComponent)
	return CurrentActiveAudioComponent;
}
