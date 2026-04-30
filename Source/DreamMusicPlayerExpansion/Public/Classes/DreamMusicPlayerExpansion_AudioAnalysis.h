// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/DreamMusicPlayerExpansion.h"
#include "DreamMusicPlayerAubio.h"
#include "DreamMusicPlayerExpansion_AudioAnalysis.generated.h"

// 前向声明
struct FLKFSResults;
class ULoudnessSettings;
class ULKFSSettings;
class UConstantQSettings;
struct FLKFSNRTResults;
struct FLoudnessResults;
struct FConstantQResults;

class UConstantQAnalyzer;
class ULKFSAnalyzer;
class ULoudnessAnalyzer;

/**
 * 音频分析拓展组件
 * 集成了 CQT (可视化)、Loudness/LKFS (响度) 和 Aubio (节拍/Onset 检测)
 */
UCLASS(DisplayName = "Audio Analysis")
class DREAMMUSICPLAYEREXPANSION_API UDreamMusicPlayerExpansion_AudioAnalysis : public UDreamMusicPlayerExpansion
{
	GENERATED_BODY()

public:
	// --- Delegates (事件委托) ---

	// CQT 频谱纹理更新完成 (用于 UI 材质显示)
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisTextureLoaded, const TArray<UTexture2D*>&, Texture);

	// 实时 CQT 数据回调
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisConstantQResult, const TArray<FConstantQResults>&, Results);

	// 实时响度数据回调
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisLoudnessResult, const FLoudnessResults&, Results);

	// 实时 LKFS 数据回调
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnalysisLKFSResult, const FLKFSResults&, Results);

	// ★ [新增] Aubio 检测到 Beat (节拍/动次打次) 时触发
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAubioBeatDetected);

	// ★ [新增] Aubio 检测到 Onset (音符起始点/任何发声瞬间) 时触发
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAubioOnsetDetected);

public:
	// --- 基础设置 ---

	// 是否自动启动 AudioBus (通常为 True)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bAutoStartAudioBus = true;

	// 用于分析的 AudioBus 资产
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	UAudioBus* AnalysisAudioBus;

	// --- 功能开关 ---

	// 启用 CQT (Constant-Q Transform) 分析 - 用于生成频谱纹理
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableConstantQAnalysis = false;

	// 启用响度分析
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableLoudnessAnalysis = false;

	// 启用 LKFS 分析
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableLKFSAnalysis = false;

	// ★ [新增] 启用 Aubio 分析 (Beat & Onset)
	// 这将在切换音乐时并在后台线程预处理音频文件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bEnableAubioAnalysis = true;

	// 是否将 CQT 结果写入 Texture2D (用于材质可视化)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings")
	bool bEnableCreateAnalysisTexture = false;
	
	// --- 详细配置对象 ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Instanced, Meta = (EditCondition = "bEnableConstantQAnalysis", EditConditionHides))
	UConstantQSettings* ConstantQSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Instanced, Meta = (EditCondition = "bEnableLoudnessAnalysis", EditConditionHides))
	ULoudnessSettings* LoudnessSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", Instanced, Meta = (EditCondition = "bEnableLKFSAnalysis", EditConditionHides))
	ULKFSSettings* LKFSSettings;

	// --- Aubio 参数 ---

	// Onset 检测阈值 (默认为 0.3)。
	// 数值越小越灵敏 (可能误检)，数值越大越严格 (可能漏检)。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Analysis|Aubio", Meta = (EditCondition = "bEnableAubioAnalysis"))
	float AubioOnsetThreshold = 0.3f;

public:
	// --- 蓝图 API ---

	UFUNCTION(BlueprintPure, Category = "Audio Analysis|Functions")
	TArray<UTexture2D*> GetAnalysisTexture() { return Internal_AnalysisTextures; }

	UFUNCTION(BlueprintPure, Category = "Audio Analysis|Functions")
	UTexture2D* GetAnalysisTextureAtChannel(int32 Channel) { return Internal_AnalysisTextures.IsValidIndex(Channel) ? Internal_AnalysisTextures[Channel] : nullptr; }

	// 获取当前歌曲的整体 BPM (由 Aubio 分析得出)
	UFUNCTION(BlueprintPure, Category = "Audio Analysis|Aubio")
	float GetCurrentBPM() const { return CachedAnalysisResult.Bpm; }

	// 获取当前歌曲的平均音高 (Hz)
	UFUNCTION(BlueprintPure, Category = "Audio Analysis|Aubio")
	float GetAveragePitch() const { return CachedAnalysisResult.AveragePitch; }

	// --- 事件分配器 ---

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisTextureLoaded OnAnalysisTextureLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisConstantQResult OnAnalysisConstantQResult;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisLoudnessResult OnAnalysisLoudnessResult;

	UPROPERTY(BlueprintAssignable, Category = "Delegate")
	FOnAnalysisLKFSResult OnAnalysisLKFSResult;

	// 节拍事件
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis|Aubio")
	FOnAubioBeatDetected OnBeatDetected;

	// 音符/起音事件
	UPROPERTY(BlueprintAssignable, Category = "Audio Analysis|Aubio")
	FOnAubioOnsetDetected OnOnsetDetected;

protected:
	// void SetAnalysisDataFromBuffer(const TArray<float>& PcmData, int32 SampleRate, int32 NumChannels);
	
	// --- 核心生命周期重写 ---
	virtual void BP_Initialize_Implementation(UDreamMusicPlayerComponent* InComponent) override;
	virtual void BP_MusicStart_Implementation() override;
	virtual void BP_ChangeMusic_Implementation(const FDreamMusicData& InData) override;
	// ★ 必须重写 Tick 以同步 Aubio 数据与播放进度
	virtual void BP_Tick_Implementation(const FDreamMusicTimestamp& InTimestamp, float InDeltaTime) override;

	// --- 内部逻辑 ---

	void UpdateConstantQAnalysisData();
	void UpdateTextureFromSpectrum(UTexture2D* InTexture, const TArray<float>& InSpectrumData);

	// --- 资源与分析器 ---
	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> Internal_AnalysisTextures;

	UPROPERTY(Transient)
	UConstantQAnalyzer* Internal_ConstantQAnalyzer;

	UPROPERTY(Transient)
	ULoudnessAnalyzer* Internal_LoudnessAnalyzer;

	UPROPERTY(Transient)
	ULKFSAnalyzer* Internal_LKFSAnalyzer;

	bool bIsCreated = false;
	int32 Internal_ChannelNums;

	// CQT 数据缓冲
	TArray<FConstantQResults> Internal_ConstantQResultBuffer;
	FCriticalSection DataGuard; // 线程锁

	// --- Aubio 数据缓存 ---

	// 存储完整的分析结果 (BPM, BeatTimes, OnsetTimes)
	FDreamMusicAnalysisResult CachedAnalysisResult;

	// 标记后台分析是否完成
	bool bIsAnalysisReady = false;

	// 当前播放进度对应的索引
	int32 CurrentBeatIndex = 0;
	int32 CurrentOnsetIndex = 0;

	// 执行异步分析任务
	void PerformAsyncAubioAnalysis(USoundWave* InSoundWave);

	// --- 内部回调 ---
	UFUNCTION()
	void OnAnalysisConstantQResults(UConstantQAnalyzer* Analyzer, int32 ChannelIndex, const FConstantQResults& Results);
	UFUNCTION()
	void OnAnalysisLoudnessResults(const FLoudnessResults& Results);
	UFUNCTION()
	void OnAnalysisLKFSResults(ULKFSAnalyzer* Analyzer, const FLKFSResults& Results);
};

// 工具类
UCLASS(BlueprintType)
class UDreamMusicPlayerExpansion_AudioAnalysisUtil : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void ModifyAudioSoundBus(UAudioBus* InBus, USoundWave* InSoundWave);
};
