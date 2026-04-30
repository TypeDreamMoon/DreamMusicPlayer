#include "Classes/DreamMusicPlayerExpansion_AudioAnalysis.h"
#include "AudioMixerBlueprintLibrary.h"
#include "ConstantQ.h"
#include "Classes/DreamMusicPlayerComponent.h"
#include "DreamMusicPlayerDebugLog.h"
#include "DreamMusicPlayerLog.h"
#include "LKFS.h"
#include "Loudness.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "Async/Async.h"
#include "Interfaces/IAudioFormat.h"

// ============================================================================
// CQT & Texture Visualization Logic
// ============================================================================

void UDreamMusicPlayerExpansion_AudioAnalysis::UpdateConstantQAnalysisData()
{
	if (!bEnableConstantQAnalysis || !Internal_ConstantQAnalyzer)
	{
		return;
	}

	if (!bEnableCreateAnalysisTexture || !bIsCreated || Internal_AnalysisTextures.IsEmpty())
	{
		return;
	}

	// 线程锁：防止音频线程正在写入 Buffer 时我们进行读取
	FScopeLock Lock(&DataGuard);

	// 确保 Buffer 数据量匹配声道数
	if (Internal_ConstantQResultBuffer.Num() < Internal_ChannelNums)
	{
		return;
	}

	// 更新纹理像素
	for (int32 ChannelIdx = 0; ChannelIdx < Internal_ChannelNums; ++ChannelIdx)
	{
		if (!Internal_AnalysisTextures.IsValidIndex(ChannelIdx) || !Internal_ConstantQResultBuffer.IsValidIndex(ChannelIdx))
		{
			continue;
		}

		UTexture2D* Texture = Internal_AnalysisTextures[ChannelIdx];
		const TArray<float>& SpectrumData = Internal_ConstantQResultBuffer[ChannelIdx].SpectrumValues;

		if (Texture && !SpectrumData.IsEmpty())
		{
			UpdateTextureFromSpectrum(Texture, SpectrumData);
		}
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::UpdateTextureFromSpectrum(UTexture2D* InTexture, const TArray<float>& InSpectrumData)
{
	if (!InTexture || InSpectrumData.IsEmpty()) return;

	FTexturePlatformData* PlatformData = InTexture->GetPlatformData();
	if (!PlatformData || !PlatformData->Mips.IsValidIndex(0)) return;

	const int32 Width = PlatformData->SizeX;
	// 防止数组越界
	const int32 BandsToCopy = FMath::Min(Width, InSpectrumData.Num());

	// 锁定纹理内存进行写入
	void* TextureDataPtr = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	if (!TextureDataPtr) return;

	uint8* DestPixels = static_cast<uint8*>(TextureDataPtr);

	for (int32 i = 0; i < BandsToCopy; ++i)
	{
		float Value = InSpectrumData[i];
		// 简单的钳制，确保颜色在 0-1 之间 (根据需要可能需要 log 缩放)
		Value = FMath::Clamp(Value, 0.0f, 1.0f);
		uint8 ByteValue = static_cast<uint8>(Value * 255.0f);

		const int32 PixelIdx = i * 4;
		DestPixels[PixelIdx + 0] = ByteValue; // B
		DestPixels[PixelIdx + 1] = ByteValue; // G
		DestPixels[PixelIdx + 2] = ByteValue; // R
		DestPixels[PixelIdx + 3] = 255; // A (不透明)
	}

	// 填充剩余部分为黑色
	if (BandsToCopy < Width)
	{
		const int32 RemainingBytes = (Width - BandsToCopy) * 4;
		FMemory::Memzero(DestPixels + (BandsToCopy * 4), RemainingBytes);
	}

	PlatformData->Mips[0].BulkData.Unlock();
	InTexture->UpdateResource();
}

// ============================================================================
// Lifecycle Methods
// ============================================================================

/*
void UDreamMusicPlayerExpansion_AudioAnalysis::SetAnalysisDataFromBuffer(const TArray<float>& PcmData, int32 SampleRate, int32 NumChannels)
{
	// 重置状态
	Internal_ChannelNums = NumChannels;
	BP_MusicStart_Implementation(); // 重新初始化 Buffer 和 Texture

	// 此处调用你的 Aubio 封装类进行离线分析
	// 假设你有一个 FDreamMusicPlayerAubio 实例或者静态方法
	// 示例：
	// FDreamMusicAnalysisResult Result;
	// AubioAnalyzer->AnalyzeRawFloatBuffer(PcmData, SampleRate, NumChannels, Result);
	// CachedAnalysisResult = Result;
	// bIsAnalysisReady = true;
}*/

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_Initialize_Implementation(UDreamMusicPlayerComponent* InComponent)
{
	if (!InComponent) return;

	UWorld* ThisWorld = InComponent->GetWorld();
	if (!ThisWorld)
	{
		DMP_LOG_DEBUG_EXPANSION(Error, TEXT("World is nullptr"));
		return;
	}

	const FAudioDeviceHandle AudioDevice = ThisWorld->GetAudioDevice();
	if (!AudioDevice.IsValid())
	{
		DMP_LOG_DEBUG_EXPANSION(Error, TEXT("AudioDevice is nullptr"));
		return;
	}

	// 启动 AudioBus
	if (bAutoStartAudioBus)
	{
		UAudioMixerBlueprintLibrary::StartAudioBus(InComponent, AnalysisAudioBus);
	}

	// 初始化 CQT 分析器
	if (bEnableConstantQAnalysis)
	{
		Internal_ConstantQAnalyzer = NewObject<UConstantQAnalyzer>(this);
		Internal_ConstantQAnalyzer->Settings = ConstantQSettings;
		Internal_ConstantQAnalyzer->OnLatestConstantQResultsNative.AddUObject(this, &UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisConstantQResults);
		Internal_ConstantQAnalyzer->StartAnalyzing(AudioDevice.GetDeviceID(), AnalysisAudioBus);
	}

	// 初始化响度分析器
	if (bEnableLoudnessAnalysis)
	{
		Internal_LoudnessAnalyzer = NewObject<ULoudnessAnalyzer>(this);
		Internal_LoudnessAnalyzer->Settings = LoudnessSettings;
		Internal_LoudnessAnalyzer->OnLatestOverallLoudnessResults.AddUniqueDynamic(this, &UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisLoudnessResults);
		Internal_LoudnessAnalyzer->StartAnalyzing(AudioDevice.GetDeviceID(), AnalysisAudioBus);
	}

	// 初始化 LKFS 分析器
	if (bEnableLKFSAnalysis)
	{
		Internal_LKFSAnalyzer = NewObject<ULKFSAnalyzer>(this);
		Internal_LKFSAnalyzer->Settings = LKFSSettings;
		Internal_LKFSAnalyzer->OnLatestOverallLKFSResultsNative.AddUObject(this, &UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisLKFSResults);
		Internal_LKFSAnalyzer->StartAnalyzing(AudioDevice.GetDeviceID(), AnalysisAudioBus);
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_MusicStart_Implementation()
{
	// 重置 Aubio 播放状态
	CurrentBeatIndex = 0;
	CurrentOnsetIndex = 0;

	// 清理旧纹理
	if (!Internal_AnalysisTextures.IsEmpty())
	{
		for (UTexture2D* Tex : Internal_AnalysisTextures)
		{
			if (Tex)
			{
				Tex->RemoveFromRoot();
				Tex->MarkAsGarbage();
			}
		}
		Internal_AnalysisTextures.Empty();
	}

	bIsCreated = false;

	// 创建分析纹理 (用于 UI 可视化)
	if (bEnableCreateAnalysisTexture)
	{
		const int32 TextureWidth = (ConstantQSettings && ConstantQSettings->NumBands > 0) ? ConstantQSettings->NumBands : 48;
		const int32 TextureHeight = 1;

		for (int i = 0; i < Internal_ChannelNums; ++i)
		{
			// 创建瞬态纹理 (不会被保存到磁盘)
			if (UTexture2D* CacheTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_B8G8R8A8))
			{
				CacheTexture->MipGenSettings = TMGS_NoMipmaps;
				CacheTexture->Filter = TF_Nearest;
				CacheTexture->CompressionSettings = TC_VectorDisplacementmap;
				CacheTexture->SRGB = 0; // 线性空间
				CacheTexture->AddToRoot(); // 防止 GC
				CacheTexture->UpdateResource();
				Internal_AnalysisTextures.Add(CacheTexture);
			}
			else
			{
				DMP_LOG_DEBUG_EXPANSION(Log, TEXT("Failed to create texture for channel %d"), i);
			}
		}

		if (Internal_AnalysisTextures.Num() == Internal_ChannelNums)
		{
			bIsCreated = true;
			OnAnalysisTextureLoaded.Broadcast(Internal_AnalysisTextures);
		}
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_ChangeMusic_Implementation(const FDreamMusicData& InData)
{
	USoundWave* SoundWave = nullptr;
	if (InData.Music.IsValid())
	{
		SoundWave = Cast<USoundWave>(InData.Music.Get());
		if (SoundWave)
		{
			Internal_ChannelNums = SoundWave->NumChannels;
			if (Internal_ChannelNums <= 0) Internal_ChannelNums = 2;
		}
	}

	// ★ 核心逻辑变更：启动 Aubio 异步分析
	if (bEnableAubioAnalysis && SoundWave)
	{
		PerformAsyncAubioAnalysis(SoundWave);
	}
	else
	{
		// 如果禁用或无效，清空状态
		bIsAnalysisReady = false;
		CachedAnalysisResult = FDreamMusicAnalysisResult();
	}
}

// ============================================================================
// Aubio Async Analysis & Tick Logic
// ============================================================================

void UDreamMusicPlayerExpansion_AudioAnalysis::PerformAsyncAubioAnalysis(USoundWave* InSoundWave)
{
	if (!InSoundWave) return;

	// 1. 标记未就绪
	bIsAnalysisReady = false;
	CurrentBeatIndex = 0;
	CurrentOnsetIndex = 0;
	CachedAnalysisResult = FDreamMusicAnalysisResult();

	// ========================================================================
	// 在主线程预加载数据
	// ========================================================================
	// 我们必须在主线程触发 GetCompressedData，以确保音频数据已从 DDC 构建/加载。
	// 如果在后台线程第一次调用它，会触发 DerivedDataCache 的构建，导致断言失败崩溃。
	if (!InSoundWave->RawPCMData) // 如果没有 RawPCMData，才需要走压缩数据流程
	{
		const FName RuntimeFormat = InSoundWave->GetRuntimeFormat();
		const FPlatformAudioCookOverrides* CompressionOverrides = USoundWave::GetPlatformCompressionOverridesForCurrentPlatform();

		// 这一行会强制引擎在主线程准备好数据 (可能会有轻微卡顿，但在切换歌曲时通常可接受)
		if (InSoundWave->GetCompressedData(RuntimeFormat, CompressionOverrides) == nullptr)
		{
			DMP_LOG_DEBUG_EXPANSION(Warning, TEXT("Failed to load compressed data on GameThread for Aubio analysis."));
			// 如果主线程都拿不到数据，后台线程更拿不到，直接返回避免崩溃
			return;
		}
	}
	// ========================================================================

	float Threshold = AubioOnsetThreshold;

	int NumChannels = InSoundWave->NumChannels;
	int SampleRate = InSoundWave->GetImportedSampleRate();
	TArray<uint8> PCMData;

	FDreamMusicPlayerAubio Analyzer;
	Analyzer.SetOnsetThreshold(Threshold);
	Analyzer.DecodeSoundWave(InSoundWave, PCMData, SampleRate, NumChannels);

	FDreamMusicAnalysisResult TempResult;

	// 这里的 AnalyzeEntireSoundWave 内部会再次调用 GetCompressedData，
	// 但因为我们在主线程已经调用过一次，这里会直接返回指针，不会崩溃。
	bool bSuccess = Analyzer.AnalyzeEntireSoundWave(PCMData, SampleRate, NumChannels, TempResult);


	if (bSuccess)
	{
		CachedAnalysisResult = TempResult;
		bIsAnalysisReady = true;
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::BP_Tick_Implementation(const FDreamMusicTimestamp& InTimestamp, float InDeltaTime)
{
	Super::BP_Tick_Implementation(InTimestamp, InDeltaTime);

	// 如果分析没开启或没完成，直接返回
	if (!bEnableAubioAnalysis || !bIsAnalysisReady) return;

	// 获取播放器当前精确时间 (秒)
	float CurrentTime = InTimestamp.ToSeconds();

	// --- 处理 Beat 触发 ---
	// 使用 while 是为了防止帧率过低导致一帧内跳过了多个 Beat
	while (CachedAnalysisResult.BeatTimes.IsValidIndex(CurrentBeatIndex))
	{
		float NextBeatTime = CachedAnalysisResult.BeatTimes[CurrentBeatIndex];

		// 如果当前时间已经超过了预记的 Beat 时间
		if (CurrentTime >= NextBeatTime)
		{
			// 触发事件
			if (OnBeatDetected.IsBound())
			{
				OnBeatDetected.Broadcast();
			}
			CurrentBeatIndex++;
		}
		else
		{
			// 还没到时间，跳出
			break;
		}
	}

	// --- 处理 Onset 触发 ---
	while (CachedAnalysisResult.OnsetTimes.IsValidIndex(CurrentOnsetIndex))
	{
		float NextOnsetTime = CachedAnalysisResult.OnsetTimes[CurrentOnsetIndex];

		if (CurrentTime >= NextOnsetTime)
		{
			if (OnOnsetDetected.IsBound())
			{
				OnOnsetDetected.Broadcast();
			}
			CurrentOnsetIndex++;
		}
		else
		{
			break;
		}
	}

	// --- 处理 Seek (进度条拖动/倒带) ---
	// 简单的检测：如果当前播放时间突然变小了 (比上一个 Beat 时间还小)，说明发生了 Seek
	if (CurrentBeatIndex > 0 && CurrentTime < CachedAnalysisResult.BeatTimes[CurrentBeatIndex - 1])
	{
		// 重置索引并快速前向搜索到正确位置
		CurrentBeatIndex = 0;
		while (CachedAnalysisResult.BeatTimes.IsValidIndex(CurrentBeatIndex) && CachedAnalysisResult.BeatTimes[CurrentBeatIndex] < CurrentTime)
		{
			CurrentBeatIndex++;
		}

		CurrentOnsetIndex = 0;
		while (CachedAnalysisResult.OnsetTimes.IsValidIndex(CurrentOnsetIndex) && CachedAnalysisResult.OnsetTimes[CurrentOnsetIndex] < CurrentTime)
		{
			CurrentOnsetIndex++;
		}
	}
}

// ============================================================================
// Callbacks & Utils
// ============================================================================

void UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisConstantQResults(UConstantQAnalyzer* Analyzer, int32 ChannelIndex, const FConstantQResults& Results)
{
	// 音频线程回调，使用 ScopeLock 保护共享数据
	FScopeLock Lock(&DataGuard);

	if (Internal_ChannelNums == 0 && Internal_ConstantQResultBuffer.IsEmpty())
	{
		return;
	}

	// 更新缓存数据
	if (Internal_ConstantQResultBuffer.Num() != Internal_ChannelNums)
	{
		Internal_ConstantQResultBuffer.SetNum(Internal_ChannelNums);
	}

	if (!Internal_ConstantQResultBuffer.IsValidIndex(ChannelIndex))
	{
		return;
	}

	Internal_ConstantQResultBuffer[ChannelIndex] = Results;

	// 更新纹理 (Tick 驱动)
	UpdateConstantQAnalysisData();

	// 广播最后一个声道的数据 (如果你需要蓝图接收原始数据)
	if (ChannelIndex == Internal_ChannelNums - 1)
	{
		if (OnAnalysisConstantQResult.IsBound())
		{
			TArray<FConstantQResults> BroadcastBuffer = Internal_ConstantQResultBuffer;
			// 转到游戏线程广播，避免 UI 线程冲突
			AsyncTask(ENamedThreads::GameThread, [this, BroadcastBuffer]()
			{
				OnAnalysisConstantQResult.Broadcast(BroadcastBuffer);
			});
		}
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisLoudnessResults(const FLoudnessResults& Results)
{
	if (OnAnalysisLoudnessResult.IsBound())
	{
		// Loudness 库的 Delegate 通常已经在 GameThread，但为了保险起见直接转发
		OnAnalysisLoudnessResult.Broadcast(Results);
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisLKFSResults(ULKFSAnalyzer* Analyzer, const FLKFSResults& Results)
{
	if (OnAnalysisLKFSResult.IsBound())
	{
		FScopeLock Lock(&DataGuard);
		AsyncTask(ENamedThreads::GameThread, [this, Results]()
		{
			OnAnalysisLKFSResult.Broadcast(Results);
		});
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysisUtil::ModifyAudioSoundBus(UAudioBus* InBus, USoundWave* InSoundWave)
{
	if (!InSoundWave || !InBus) return;

	InSoundWave->bEnableBusSends = true;

	for (const auto& BusSendInfo : InSoundWave->BusSends)
	{
		if (BusSendInfo.AudioBus == InBus)
		{
			return;
		}
	}

	FSoundSourceBusSendInfo BusSendInfo;
	BusSendInfo.AudioBus = InBus;
	BusSendInfo.SendLevel = 1.0f;

	InSoundWave->BusSends.Add(BusSendInfo);
	InSoundWave->MarkPackageDirty();
}
