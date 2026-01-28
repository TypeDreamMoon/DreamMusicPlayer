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

	// 线程安全锁：防止音频线程正在写入Buffer时我们进行读取
	FScopeLock Lock(&DataGuard);

	// 确保Buffer数据量匹配声道数，否则说明数据还没准备好
	if (Internal_ConstantQResultBuffer.Num() < Internal_ChannelNums)
	{
		return;
	}

	// 使用 Buffer 进行鼓点分析 (通常只分析声道0或混合声道)
	if (bIsBufferInitialized && Internal_ChannelHistoryBuffers.IsValidIndex(0))
	{
		ProcessKickDetectionWithBuffer(0);
	}

	for (int32 ChannelIdx = 0; ChannelIdx < Internal_ChannelNums; ++ChannelIdx)
	{
		// 安全检查：索引有效性
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

	if (bAutoStartAudioBus)
	{
		UAudioMixerBlueprintLibrary::StartAudioBus(InComponent, AnalysisAudioBus);
	}

	if (bEnableConstantQAnalysis)
	{
		Internal_ConstantQAnalyzer = NewObject<UConstantQAnalyzer>(this);
		Internal_ConstantQAnalyzer->Settings = ConstantQSettings;
		Internal_ConstantQAnalyzer->OnLatestConstantQResultsNative.AddUObject(this, &UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisConstantQResults);
		Internal_ConstantQAnalyzer->StartAnalyzing(AudioDevice.GetDeviceID(), AnalysisAudioBus);
	}
	if (bEnableLoudnessAnalysis)
	{
		Internal_LoudnessAnalyzer = NewObject<ULoudnessAnalyzer>(this);
		Internal_LoudnessAnalyzer->Settings = LoudnessSettings;
		Internal_LoudnessAnalyzer->OnLatestOverallLoudnessResults.AddUniqueDynamic(this, &UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisLoudnessResults);
		Internal_LoudnessAnalyzer->StartAnalyzing(AudioDevice.GetDeviceID(), AnalysisAudioBus);
	}
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
	// 1. 清理旧数据
	{
		FScopeLock Lock(&DataGuard);

		// 1. 初始化历史缓冲区数组
		Internal_ChannelHistoryBuffers.Empty();
		Internal_ChannelHistoryBuffers.SetNum(Internal_ChannelNums);

		// 2. 为每个声道分配环形内存
		// 注意：ConstantQSettings->NumBands 必须有效
		int32 NumBands = (ConstantQSettings && ConstantQSettings->NumBands > 0) ? ConstantQSettings->NumBands : 48;

		for (int32 i = 0; i < Internal_ChannelNums; ++i)
		{
			Internal_ChannelHistoryBuffers[i].Resize(HistoryBufferSize, NumBands);
		}

		bIsBufferInitialized = true;
	}

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

	// 2. 重新创建纹理
	if (bEnableCreateAnalysisTexture)
	{
		// 关键修复：纹理宽度应该是频段数量 (NumBands)，而不是 48 硬编码，也不是声道数
		// 如果 ConstantQSettings.NumBands 是 0，默认给 48 防止崩溃
		const int32 TextureWidth = (ConstantQSettings->NumBands > 0) ? ConstantQSettings->NumBands : 48;
		const int32 TextureHeight = 1;

		for (int i = 0; i < Internal_ChannelNums; ++i)
		{
			// PF_B8G8R8A8 是最通用的格式
			if (UTexture2D* CacheTexture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_B8G8R8A8))
			{
				CacheTexture->MipGenSettings = TMGS_NoMipmaps;
				CacheTexture->Filter = TF_Nearest;
				CacheTexture->CompressionSettings = TC_VectorDisplacementmap; // 这里的压缩设置通常用 VectorDisplacementMap 或 UserInterface2D 来保持原始值
				CacheTexture->SRGB = 0; // 线性颜色
				CacheTexture->AddToRoot(); // 防止被 GC
				CacheTexture->UpdateResource();

				// 关键修复：使用 Add 而不是下标访问空数组
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
	if (InData.Music.IsValid())
	{
		if (USoundWave* SoundWave = Cast<USoundWave>(InData.Music.Get()))
		{
			Internal_ChannelNums = SoundWave->NumChannels;
			// 防御性代码：防止 SoundWave 返回 0 声道
			if (Internal_ChannelNums <= 0) Internal_ChannelNums = 2;
		}
	}
}


void UDreamMusicPlayerExpansion_AudioAnalysis::UpdateTextureFromSpectrum(UTexture2D* InTexture, const TArray<float>& InSpectrumData)
{
	if (!InTexture || InSpectrumData.IsEmpty()) return;

	FTexturePlatformData* PlatformData = InTexture->GetPlatformData();
	if (!PlatformData || !PlatformData->Mips.IsValidIndex(0)) return;

	const int32 Width = PlatformData->SizeX;
	const int32 Height = PlatformData->SizeY;

	// 确保数据长度不超过纹理宽度
	const int32 BandsToCopy = FMath::Min(Width, InSpectrumData.Num());

	// 锁定纹理内存
	void* TextureDataPtr = PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	if (!TextureDataPtr) return;

	// 直接操作指针，比 new uint8[] 再 memcpy 更快
	uint8* DestPixels = static_cast<uint8*>(TextureDataPtr);

	// 遍历频段数据
	for (int32 i = 0; i < BandsToCopy; ++i)
	{
		// 简单的可视化映射：将 float 幅度 (0.0 - 1.0+) 映射到 0-255
		// ConstantQ 的值通常比较小，可能需要乘以一个强度系数，比如 255.0f
		// 这里假设数据已经归一化，或者你可以添加一个 Multiplier
		float Value = InSpectrumData[i];

		// 钳制到 0-1
		Value = FMath::Clamp(Value, 0.0f, 1.0f);

		uint8 ByteValue = static_cast<uint8>(Value * 255.0f);

		// 填充 BGRA (4 bytes per pixel)
		const int32 PixelIdx = i * 4;
		DestPixels[PixelIdx + 0] = ByteValue; // B
		DestPixels[PixelIdx + 1] = ByteValue; // G
		DestPixels[PixelIdx + 2] = ByteValue; // R
		DestPixels[PixelIdx + 3] = 255; // A (Fully Opaque)
	}

	// 如果纹理比数据宽（比如 NumBands < TextureWidth），把剩余部分填黑
	if (BandsToCopy < Width)
	{
		const int32 RemainingBytes = (Width - BandsToCopy) * 4;
		FMemory::Memzero(DestPixels + (BandsToCopy * 4), RemainingBytes);
	}

	PlatformData->Mips[0].BulkData.Unlock();
	InTexture->UpdateResource();
}

// 移除了 BuildPixelArray 和 WriteTextureFromPixel，功能合并到了 UpdateTextureFromSpectrum 以提高效率和安全性

void UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisConstantQResults(UConstantQAnalyzer* Analyzer, int32 ChannelIndex, const FConstantQResults& Results)
{
	// 音频线程回调，加锁保护 Buffer
	FScopeLock Lock(&DataGuard);

	if (!bIsBufferInitialized || !Internal_ChannelHistoryBuffers.IsValidIndex(ChannelIndex))
	{
		return;
	}

	// 1. 将新数据 Push 进该声道的环形缓冲区
	Internal_ChannelHistoryBuffers[ChannelIndex].PushFrame(Results.SpectrumValues);

	// 2. 将“最新”的一帧也存一份到原来的 Buffer 里 (为了兼容你之前的 Texture 更新逻辑)
	if (Internal_ConstantQResultBuffer.Num() != Internal_ChannelNums)
	{
		Internal_ConstantQResultBuffer.SetNum(Internal_ChannelNums);
	}
	Internal_ConstantQResultBuffer[ChannelIndex] = Results;

	// 3. 触发后续逻辑
	UpdateConstantQAnalysisData();

	// 最后一个声道数据到达时，广播事件（如果是给蓝图用的）
	// 注意：Texture 的更新是在 Tick (UpdateConstantQAnalysisData) 里做的，不依赖这个广播
	if (ChannelIndex == Internal_ChannelNums - 1)
	{
		// 为了线程安全，这里广播可能会有问题，建议仅在 GameThread (Tick) 中广播
		// 如果必须在这里广播，确保接收端不修改 UI
		if (OnAnalysisConstantQResult.IsBound())
		{
			// 复制一份数据广播，避免引用被修改
			TArray<FConstantQResults> BroadcastBuffer = Internal_ConstantQResultBuffer;
			// 必须在游戏线程广播 Delegate
			AsyncTask(ENamedThreads::GameThread, [this, BroadcastBuffer]()
			{
				OnAnalysisConstantQResult.Broadcast(BroadcastBuffer);
			});
		}
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisLoudnessResults(const FLoudnessResults& Results)
{
	OnAnalysisLoudnessResult.Broadcast(Results);
}

void UDreamMusicPlayerExpansion_AudioAnalysis::OnAnalysisLKFSResults(ULKFSAnalyzer* Analyzer, const FLKFSResults& Results)
{
	// 音频线程回调，加锁保护 Buffer
	FScopeLock Lock(&DataGuard);

	if (OnAnalysisLKFSResult.IsBound())
	{
		// 必须在游戏线程广播 Delegate
		AsyncTask(ENamedThreads::GameThread, [this, Results]()
		{
			OnAnalysisLKFSResult.Broadcast(Results);
		});
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysis::ProcessKickDetectionWithBuffer(int32 ChannelIndex)
{
	FSpectrumRingBuffer& RingBuffer =  Internal_ChannelHistoryBuffers[ChannelIndex];

	// 1. 获取最新一帧的数据 (WriteIndex - 1 也就是刚写进去的那帧)
	int32 LatestIdx = (RingBuffer.WriteIndex - 1 + RingBuffer.MaxHistorySize) % RingBuffer.MaxHistorySize;
	const TArray<float>& CurrentSpectrum = RingBuffer.Buffer[LatestIdx];

	if (CurrentSpectrum.IsEmpty()) return;

	// 设定低频范围 (假设前 20 个频段是低音)
	int32 LowBandEnd = FMath::Min(CurrentSpectrum.Num(), 20) - 1;

	// 2. 计算【当前瞬间】的低频能量
	float CurrentEnergy = 0.0f;
	for (int32 i = 0; i <= LowBandEnd; ++i)
	{
		CurrentEnergy += CurrentSpectrum[i];
	}
	CurrentEnergy /= (LowBandEnd + 1); // 求平均

	// 3. 计算【过去 X 帧】的局部平均能量 (Moving Average)
	// 比如：对比过去 40 帧 (约 0.6秒) 的平均值
	// 这种对比方式比 Lerp 更能反映“当前的背景音量水平”
	float AverageHistoryEnergy = RingBuffer.GetAverageEnergyOfRecentFrames(40, 0, LowBandEnd);

	// 4. 触发判定
	// 算法：当前能量 > 局部平均能量 * 系数C + 偏置B
	// C (BeatSensitivity): 比如 1.4，表示要突出的多明显
	// B (Bias): 比如 0.01，防止静音时的噪音触发

	bool bIsCoolDown = (GetWorld()->GetTimeSeconds() - LastBeatTriggerTime) < BeatCooldownDuration;
	bool bIsBeat = CurrentEnergy > (AverageHistoryEnergy * BeatSensitivity) + 0.01f;

	if (bIsBeat && !bIsCoolDown)
	{
		LastBeatTriggerTime = GetWorld()->GetTimeSeconds();

		// 归一化强度
		float Intensity = FMath::Clamp((CurrentEnergy / (AverageHistoryEnergy + 0.001f)) - 1.0f, 0.0f, 1.0f);

		// 广播
		AsyncTask(ENamedThreads::GameThread, [this, Intensity]()
		{
			OnKickDetected.Broadcast(Intensity);
		});
	}
}

void UDreamMusicPlayerExpansion_AudioAnalysisUtil::ModifyAudioSoundBus(UAudioBus* InBus, USoundWave* InSoundWave)
{
	if (!InSoundWave || !InBus) return;

	InSoundWave->bEnableBusSends = true;

	// Check Has This AudioBus
	for (const auto& BusSendInfo : InSoundWave->BusSends)
	{
		if (BusSendInfo.AudioBus == InBus)
		{
			return;
		}
	}

	// If Not has, Add
	FSoundSourceBusSendInfo BusSendInfo;
	BusSendInfo.AudioBus = InBus;
	// 建议设置发送级别，确保有信号发送
	BusSendInfo.SendLevel = 1.0f;

	InSoundWave->BusSends.Add(BusSendInfo);
	InSoundWave->MarkPackageDirty();
}
