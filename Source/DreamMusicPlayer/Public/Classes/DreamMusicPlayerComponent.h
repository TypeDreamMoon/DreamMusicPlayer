// Copyright © Dream Moon Studio . Dream Moon All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamMusicPlayerComponent.generated.h"

class UConstantQNRTSettings;
class ULoudnessNRTSettings;
class UDreamAsyncAction_KMeansTexture;
/**
 * 
 */
UCLASS(ClassGroup=DreamComponent, Blueprintable, meta=(BlueprintSpawnableComponent), HideCategories=(Parameters))
class DREAMMUSICPLAYER_API UDreamMusicPlayerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

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

	UPROPERTY(BlueprintAssignable, Category = "Delegates|MusicData")
	FMusicPlayerMusicDataDelegate OnMusicDataChanged;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|MusicData")
	FMusicPlayerMusicDataListDelegate OnMusicDataListChanged;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerMusicDataDelegate OnMusicPlay;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerCommonDelegate OnMusicPause;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerCommonDelegate OnMusicUnPause;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerCommonDelegate OnMusicEnd;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Lyric")
	FMusicPlayerLyircListDelegate OnLyricListChanged;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Music")
	FMusicPlayerTick OnMusicTick;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|State")
	FMusicPlayerPlayStateDelegate OnPlayStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|State")
	FMusicPlayerPlayModeDelegate OnPlayModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Delegates|Lyric")
	FMusicPlayerLyricAndIndexDelegate OnLyricChanged;

public:
	/** Propertys **/

	// Components

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SubAudioComponentA = nullptr;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> SubAudioComponentB = nullptr;

	// State

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	bool bIsPlaying = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	bool bIsPaused = false;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	EDreamMusicPlayerPlayMode PlayMode = EDreamMusicPlayerPlayMode::EDMPPS_Normal;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	EDreamMusicPlayerPlayState PlayState = EDreamMusicPlayerPlayState::EDMPPS_Stop;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentDuration = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FDreamMusicDataStruct CurrentMusicData;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TArray<FDreamMusicLyric> CurrentMusicLyricList;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FDreamMusicLyric CurrentLyric;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentMusicDuration = 0.0f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<USoundWave> SoundWave;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<UConstantQNRT> ConstantQ;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<ULoudnessNRT> Loudness;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TObjectPtr<UTexture2D> Cover;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TArray<float> ConstantQDataL;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	TArray<float> ConstantQDataR;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float LoudnessValue;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	float CurrentMusicPercent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	FDreamMusicLyricTimestamp CurrentTimestamp;

	// If ture SubB else SubA
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "State")
	bool CurrentActiveAudioComponent = false;

	// Data And Settings

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TObjectPtr<UDataTable> SondList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<FDreamMusicDataStruct> MusicDataList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float UpdateTime = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float LyricOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int CoverThemeColorCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int MaxIterationsCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FDreamMusicPlayerFadeAudioSetting FadeAduioSetting;

private:
	FTimerHandle TimerHandle;
	FTimerHandle StopTimerHandle;
	UPROPERTY()
	UDreamAsyncAction_KMeansTexture* Action_KMeansTexture;

public:
	// 设置播放模式
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetPlayMode(EDreamMusicPlayerPlayMode InPlayMode);

	// 初始化歌词
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeLyricList();

	// 初始化列表
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeMusicList();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeMusicListWithSongTable(UDataTable* Table);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void InitializeMusicListWithDataArray(TArray<FDreamMusicDataStruct> InData);

	// 播放音乐
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusic(EDreamMusicPlayerPlayMode InPlayMode);

	// 播放下一首
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayNextMusic();

	// 播放上一首
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayLastMusic();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusicWithLyric(FDreamMusicLyric InLyric);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetPauseMusic(bool bInPause);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void TogglePauseMusic();

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetMusicPercent(float InPercent);

	// 从音乐数据结构播放音乐
	// Loop
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusicFromMusicData(FDreamMusicDataStruct InData);

	// 从音乐数据播放音乐
	// Loop
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void PlayMusicFromMusicDataAsset(UDreamMusicData* InData);

	// 获取下一首
	UFUNCTION(BlueprintCallable, Category = "Functions")
	FDreamMusicDataStruct GetNextMusicData(FDreamMusicDataStruct InData);

	// 获取上一首
	UFUNCTION(BlueprintCallable, Category = "Functions")
	FDreamMusicDataStruct GetLastMusicData(FDreamMusicDataStruct InData);

	// 获取音频可视化数据
	UFUNCTION(BlueprintPure, Category = "Functions")
	void GetAudioNrtData(TArray<float>& ConstantNrtL, TArray<float>& ConstantNrtR, TArray<float>& ConstantNrtAverage,
	                     float& OutLoudnessValue);

	UFUNCTION(BlueprintPure, Category = "Functions")
	UAudioComponent* GetActiveAudioComponent() const;

	UFUNCTION(BlueprintPure, Category = "Functions")
	UAudioComponent* GetLastActiveAudioComponent() const;

public:
	
	UFUNCTION()
	TArray<FString> GetNames() const;
	
private:
	// 播放
	void StartMusic();

	// 结束
	void EndMusic(bool Native = false);

	// 暂停
	void PauseMusic();

	// 取消暂停
	void UnPauseMusic();

	// 设置音乐数据
	void SetMusicData(FDreamMusicDataStruct InData);

	// 设置播放状态
	void SetPlayState(EDreamMusicPlayerPlayState InState);

	// 设置当前歌词
	void SetCurrentLyric(FDreamMusicLyric InLyric);

	// 加载资源
	void LoadAsset();

	// 加载音频NRT数据
	void LoadAudioNrt();

	// 音乐帧
	void MusicTick();

	bool ToggleActiveAudioComponent();
};
