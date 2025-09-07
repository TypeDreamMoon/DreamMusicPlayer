// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamMusicPlayerComponent.generated.h"

struct FKMeansColorCluster;
class UConstantQNRTSettings;
class ULoudnessNRTSettings;
class UDreamAsyncAction_KMeansTexture;
/**
 * 
 */
UCLASS(ClassGroup=DreamComponent, Blueprintable, meta=(BlueprintSpawnableComponent),
	HideCategories=(Parameters,ComponentTick,ComponentReplication), AutoCollapseCategories=(State))
class DREAMMUSICPLAYER_API UDreamMusicPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDreamMusicPlayerComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/** Delegates **/

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerMusicDataDelegate, FDreamMusicDataStruct, Data);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerMusicDataListDelegate, TArray<FDreamMusicDataStruct>, List);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerLyircListDelegate, const TArray<FDreamMusicLyric>&,
	                                            LyricList);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerLyricDelegate, FDreamMusicLyric, Lyric);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMusicPlayerLyricAndIndexDelegate, FDreamMusicLyric, Lyric, int, Index)
	;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMusicPlayerCommonDelegate);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerPlayStateDelegate, EDreamMusicPlayerPlayState, State);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerPlayModeDelegate, EDreamMusicPlayerPlayMode, Mode);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerTick, float, Time);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMusicPlayerThemeColorChanged, const TArray<FKMeansColorCluster>&, Colors, bool, bSuccess);

	/**
	 * Music Data Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|MusicData")
	FMusicPlayerMusicDataDelegate OnMusicDataChanged;

	/**
	 * Music Data List Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|MusicData")
	FMusicPlayerMusicDataListDelegate OnMusicDataListChanged;

	/**
	 * Music Play
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerMusicDataDelegate OnMusicPlay;

	/**
	 * Music Pause
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerCommonDelegate OnMusicPause;

	/**
	 * Music Unpause
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerCommonDelegate OnMusicUnPause;

	/**
	 * Music End
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerCommonDelegate OnMusicEnd;

	/**
	 * Lyric List Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Lyric")
	FMusicPlayerLyircListDelegate OnLyricListChanged;

	/**
	 * Music Tick
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerTick OnMusicTick;

	/**
	 * Play State Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|State")
	FMusicPlayerPlayStateDelegate OnPlayStateChanged;

	/**
	 * Play Mode Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|State")
	FMusicPlayerPlayModeDelegate OnPlayModeChanged;

	/**
	 * Lyric Changed
	 */
	UPROPERTY(BlueprintAssignable, Category = "Delegates|Lyric")
	FMusicPlayerLyricAndIndexDelegate OnLyricChanged;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|ThemeColor")
	FMusicPlayerThemeColorChanged OnThemeColorChanged;

public:
	/** Propertys **/

	// A Audio Component
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SubAudioComponentA = nullptr;

	// B Audio Component
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SubAudioComponentB = nullptr;

#pragma region State

	// State

	// Current Is Playing?
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	bool bIsPlaying = false;

	// Current Is Pause?
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	bool bIsPaused = false;

	// Music Player Play Mode
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	EDreamMusicPlayerPlayMode PlayMode = EDreamMusicPlayerPlayMode::EDMPPS_Normal;

	// Music Player Play State
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	EDreamMusicPlayerPlayState PlayState = EDreamMusicPlayerPlayState::EDMPPS_Stop;

	// Current Play Duration
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentDuration = 0.0f;

	// Current Music Data Struct
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FDreamMusicDataStruct CurrentMusicData;

	// Current Music Lyric List
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TArray<FDreamMusicLyric> CurrentMusicLyricList;

	// Current Music Lyric
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FDreamMusicLyric CurrentLyric;

	// Music End Duration
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentMusicDuration = 0.0f;

	// Current Music Wave
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<USoundWave> SoundWave;

	// Current Music ConstantQ Data
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<UConstantQNRT> ConstantQ;

	// Current Music Loudness Data
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<ULoudnessNRT> Loudness;

	// Current Music Cover
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<UTexture2D> Cover;

	// Current Music Duration L Channel ConstantQ NRT Data
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TArray<float> ConstantQDataL;

	// Current Music Duration R Channel ConstantQ NRT Data
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TArray<float> ConstantQDataR;

	// Current Music Duration Loudness NRT Data
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float LoudnessValue;

	// Current Music Duration Percent
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentMusicPercent;

	// Current Music Timestamp
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FDreamMusicLyricTimestamp CurrentTimestamp;

	// If ture SubB else SubA
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	bool CurrentActiveAudioComponent = false;

#pragma endregion State

#pragma region Data

	// Song List
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta=(RequiredAssetDataTags="RowStructure=/Script/DreamMusicPlayer.DreamMusicPlayerSongList"))
	TObjectPtr<UDataTable> SongList;

	// Music Data List
	UPROPERTY(BlueprintReadWrite, Category = "Data")
	TArray<FDreamMusicDataStruct> MusicDataList;

#pragma endregion Data

#pragma region Settings

	// Lyric Offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float LyricOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bUseThemeColorExtension = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta=(EditCondition="bUseThemeColorExtension"))
	float SampleRate = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta=(EditCondition="bUseThemeColorExtension"))
	bool bIgnoreTransparent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta=(EditCondition="bUseThemeColorExtension"))
	float AlphaThreshold = 0.5f;

	// Cover Theme Color Count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta=(EditCondition="bUseThemeColorExtension"))
	int CoverThemeColorCount = 4;

	// Max Iterations Count
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta=(EditCondition="bUseThemeColorExtension"))
	int MaxIterationsCount = 3;

	// Fade Audio Setting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FDreamMusicPlayerFadeAudioSetting FadeAudioSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BackdropMusicVolume = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	USoundClass* SoundClass;

#pragma endregion Settings

private:
	FTimerHandle StopTimerHandle;

public:
	/**
	 * Set Music Player Play Mode
	 * @param InPlayMode Play Mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetPlayMode(EDreamMusicPlayerPlayMode InPlayMode);

	/**
	 * Initialize Lyric List
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeLyricList();

	/**
	 * Initialize Music List
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeMusicList();

	/**
	 * Initialize Music List With Song Table
	 * @param Table Music Song Table
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeMusicListWithSongTable(UDataTable* Table);

	/**
	 * Initialize Music LIst With Music Data Array
	 * @param InData Music Datas
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeMusicListWithDataArray(TArray<FDreamMusicDataStruct> InData);

	/**
	 * Play Music
	 * @param InPlayMode Play Mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusic(EDreamMusicPlayerPlayMode InPlayMode);

	/**
	 * Play Next Music
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayNextMusic();

	/**
	 * Play Last Music
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayLastMusic();

	/**
	 * Play Music Time From Lyric Timestamp
	 * @param InLyric Lyric
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusicWithLyric(FDreamMusicLyric InLyric);

	/**
	 * Set Music Pause
	 * @param bInPause Pause State
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetPauseMusic(bool bInPause);

	/**
	 * Toggle Music Pause
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void TogglePauseMusic();

	/**
	 * Set Music Percent
	 * @param InPercent Music Percent 0-1
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetMusicPercent(float InPercent);

	/**
	 * Set Music Percent From Timestamp
	 * @param InTimestamp Timestamp
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetMusicPercentFromTimestamp(FDreamMusicLyricTimestamp InTimestamp);

	/**
	 * Play Music From Music Data
	 * This Mode is Loop
	 * @param InData Music Data
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusicFromMusicData(FDreamMusicDataStruct InData);

	/**
	 * Play Music From Music Data
	 * This Mode is Loop
	 * @param InData Music Data
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusicFromMusicDataAsset(UDreamMusicData* InData);

	/**
	 * Get Next Music Data
	 * @param InData Current Music Data
	 * @return The Next Track Of The Music
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	FDreamMusicDataStruct GetNextMusicData(FDreamMusicDataStruct InData);

	/**
	 * Get Last Music Data
	 * @param InData Current Music Data
	 * @return The Last Track Of The Music
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	FDreamMusicDataStruct GetLastMusicData(FDreamMusicDataStruct InData);

	/**
	 * Get Current Duration Music NRT Data
	 * @param bConstantReverse Reverse Average?
	 * @param ConstantNrtL Current Music Duration L Channel NRT Data
	 * @param ConstantNrtR Current Music Duration R Channel NRT Data
	 * @param ConstantNrtAverage NRT Data After Averaging L And R Channels
	 * @param OutLoudnessValue Current Music Duration Loudness Value
	 */
	UFUNCTION(BlueprintPure, Category = "Functions")
	void GetAudioNrtData(bool bConstantReverse, TArray<float>& ConstantNrtL, TArray<float>& ConstantNrtR, TArray<float>& ConstantNrtAverage,
	                     float& OutLoudnessValue);

	/**
	 * Get Current Active Audio Component
	 * @return Current Active Audio Component
	 */
	UFUNCTION(BlueprintPure, Category = "Functions")
	UAudioComponent* GetActiveAudioComponent() const;

	/**
	 * Get Last Active Audio Component
	 * @return Last Active Audio Component
	 */
	UFUNCTION(BlueprintPure, Category = "Functions")
	UAudioComponent* GetLastActiveAudioComponent() const;

	/**
	* Get Current Lyric Word Progress for regular lyrics
	* @param InTimestamp Current playback time in seconds
	* @return Progress information for word timings
	*/
	UFUNCTION(BlueprintPure, Category = "Functions|Lyric")
	FDreamMusicLyricProgress GetCurrentLyricWordProgress(const FDreamMusicLyricTimestamp& InTimestamp) const;

	/**
	 * Get Current Romanization Word Progress
	 * @param InTimestamp Current playback time in seconds
	 * @return Progress information for romanization word timings
	 */
	UFUNCTION(BlueprintPure, Category = "Functions|Lyric")
	FDreamMusicLyricProgress GetCurrentRomanizationProgress(const FDreamMusicLyricTimestamp& InTimestamp) const;

	/**
	 * Get Current Lyric Line Progress (fallback when no word timings available)
	 * @param InTimestamp Current playback time in seconds
	 * @return Progress information based on line timestamps
	 */
	UFUNCTION(BlueprintPure, Category = "Functions|Lyric")
	FDreamMusicLyricProgress GetCurrentLyricLineProgress(const FDreamMusicLyricTimestamp& InTimestamp) const;

	/**
		 * Extract theme colors from current music cover asynchronously
		 * @param ClusterCount Number of dominant colors to extract
		 * @param MaxIterations Maximum K-Means iterations
		 */
	UFUNCTION(BlueprintCallable, Category = "Functions|Theme")
	void ExtractCoverThemeColors(int32 ClusterCount = 5, int32 MaxIterations = 100);

	/**
	 * Extract theme colors from any texture asynchronously
	 * @param Texture Target texture to analyze
	 * @param ClusterCount Number of dominant colors to extract
	 * @param MaxIterations Maximum K-Means iterations
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions|Theme")
	void ExtractTextureThemeColors(UTexture2D* Texture, int32 ClusterCount = 5, int32 MaxIterations = 100);

	/**
	 * Check if audio component is ready for use
	 */
	bool IsAudioComponentReady(UAudioComponent* Component) const;

public:
	UFUNCTION()
	TArray<FString> GetNames() const;

private:
	/**
	 * Start Music Native
	 */
	void StartMusic();

	/**
	 * End Music Native
	 * @param Native Whether To Call For A Component
	 */
	void EndMusic(bool Native = false);

	void HandleAutoPlayTransition();

	/**
	 * Pause Native
	 */
	void PauseMusic();

	/**
	 * Unpause Native
	 */
	void UnPauseMusic();

	/**
	 * Set Music Data
	 * @param InData New Music Data
	 */
	void SetMusicData(FDreamMusicDataStruct InData);

	/**
	 * Set Play State
	 * @param InState New State
	 */
	void SetPlayState(EDreamMusicPlayerPlayState InState);

	/**
	 * Set Current Lyric
	 * @param InLyric New Lyric
	 */
	void SetCurrentLyric(FDreamMusicLyric InLyric);

	/**
	 * Load NRT Data
	 */
	void LoadAudioNrt();

	/**
	 * Music Timer Tick
	 */
	void MusicTick(float DeltaTime);

	/**
	 * Toggle Active Audio Component
	 * @return Whether the A audio component is active
	 */
	bool ToggleActiveAudioComponent();

	/**
	 * Helper function to calculate word progress
	 * @param InCurrentTime Current playback time in seconds
	 * @param bUseRoma Array of word timings
	 * @return Progress information
	 */
	FDreamMusicLyricProgress CalculateWordProgress(FDreamMusicLyricTimestamp InCurrentTime, bool bUseRoma = false) const;

	/**
	 * Helper function to calculate line progress
	 * @param InCurrentTime Current playback time in seconds
	 * @return Progress information
	 */
	FDreamMusicLyricProgress CalculateLineProgress(FDreamMusicLyricTimestamp InCurrentTime) const;

	// 音乐开始播放的世界时间
	double MusicStartWorldTime = 0.0;
    
	// 最后一次 Seek 的位置
	float LastSeekPosition = 0.0f;
    
	// 是否刚刚进行了 Seek 操作
	bool bJustSeeked = false;
    
	/**
	 * 获取更精确的当前播放时间
	 */
	float GetAccuratePlayTime() const;
    
	/**
	 * 更新音频分析数据
	 */
	void UpdateAudioAnalysisData();

	mutable int32 CachedCurrentWordIndex = -1;
	mutable FDreamMusicLyricTimestamp LastCalculationTime;
	mutable TArray<int32> WordDurationPrefixSum; // 前缀和数组，提升查找性能
	mutable bool bCacheValid = false;
	mutable bool bLastUseRoma = false;

	void BuildWordDurationCache(bool bUseRoma) const;

	void ClearLyricProgressCache()
	{
		CachedCurrentWordIndex = -1;
		bCacheValid = false;
		LastCalculationTime = FDreamMusicLyricTimestamp{};
	}
	
private:
	UPROPERTY()
	TObjectPtr<UDreamAsyncAction_KMeansTexture> CurrentKMeansTask;

	UFUNCTION()
	void OnThemeColorsExtracted(const TArray<FKMeansColorCluster>& ColorClusters, bool bSuccess);
};
