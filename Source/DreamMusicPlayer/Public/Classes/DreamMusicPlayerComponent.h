// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicPlayerCommon.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerComponent.generated.h"


class UDreamMusicAudioManager;
class UDreamMusicPlayerExpansion;
class UDreamAsyncAction_KMeansTexture;
struct FKMeansColorCluster;

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
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMusicPlayerCommonDelegate);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerPlayStateDelegate, EDreamMusicPlayerPlayState, State);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerPlayModeDelegate, EDreamMusicPlayerPlayMode, Mode);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMusicPlayerTick, float, Time);

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

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Expansion")
	FMusicPlayerCommonDelegate OnExtensionInitializedCompleted;

public:
	/** Properties **/

#pragma region State

	// State

	// Current Music Wave
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<USoundWave> SoundWave;

	// Current Music Cover
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<UTexture2D> Cover;

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

	// Music End Duration
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentMusicDuration = 0.0f;

	// Current Music Duration Percent
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentMusicPercent = 0.f;

	// Current Music Timestamp
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FDreamMusicLyricTimestamp CurrentTimestamp;


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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Settings")
	UDreamMusicAudioManager* AudioManager;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Instanced, Category = "Settings")
	TArray<UDreamMusicPlayerExpansion*> ExpansionList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float BackdropMusicVolume = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings")
	USoundClass* SoundClass = nullptr;

#pragma endregion Settings

public:
	/**
	 * Set Music Player Play Mode
	 * @param InPlayMode Play Mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetPlayMode(EDreamMusicPlayerPlayMode InPlayMode);

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

	UFUNCTION(BlueprintPure, Category = "Functions|Expansion", Meta = (DeterminesOutputType="InExpansionClass", DynamicOutputParam="OutExpansion"))
	void GetExpansionByClass(TSubclassOf<UDreamMusicPlayerExpansion> InExpansionClass, UDreamMusicPlayerExpansion*& OutExpansion) const;

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
	 * Music Timer Tick
	 */
	void MusicTick(float DeltaTime);

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

public:
	template <typename T>
	T* GetExpansion() const
	{
		for (auto Expansion : ExpansionList)
		{
			if (Expansion->IsA(T::StaticClass()))
			{
				return Cast<T>(Expansion);
			}
		}
		return nullptr;
	}
};
