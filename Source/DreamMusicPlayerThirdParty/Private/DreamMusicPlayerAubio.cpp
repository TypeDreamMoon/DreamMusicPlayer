// Copyright Dream Moon. All Rights Reserved.

#include "DreamMusicPlayerAubio.h"

// UE 音频引擎相关头文件
#include "AudioDevice.h"            // 用于创建解码器
#include "AudioDecompress.h"        // 用于解压音频
#include "Interfaces/IAudioFormat.h"


THIRD_PARTY_INCLUDES_START
#ifndef AUBIO_UNSTABLE
#define AUBIO_UNSTABLE 0
#endif
// 引入 Aubio C 语言库
extern "C" {
#include "aubio/aubio.h"
}

THIRD_PARTY_INCLUDES_END

// 构造函数：初始化指针为空
FDreamMusicPlayerAubio::FDreamMusicPlayerAubio()
	: AubioTempoObj(nullptr)
	  , AubioPitchObj(nullptr)
	  , AubioOnsetObj(nullptr)
	  , InputBuffer(nullptr)
	  , OutputBuffer(nullptr)
	  , OnsetOutputBuffer(nullptr)
	  , CachedSampleRate(44100)
	  , CachedBufferSize(1024)
	  , CachedHopSize(512)
	  , CurrentBPM(0.0f)
	  , CurrentPitch(0.0f)
	  , CurrentConfidence(0.0f)
	  , OnsetThreshold(0.3f) // 默认阈值，适合大多数流行音乐
{
}

// 析构函数：确保资源释放
FDreamMusicPlayerAubio::~FDreamMusicPlayerAubio()
{
	Cleanup();
}

void FDreamMusicPlayerAubio::Cleanup()
{
	// 释放 Tempo 对象
	if (AubioTempoObj)
	{
		del_aubio_tempo((aubio_tempo_t*)AubioTempoObj);
		AubioTempoObj = nullptr;
	}

	// 释放 Pitch 对象
	if (AubioPitchObj)
	{
		del_aubio_pitch((aubio_pitch_t*)AubioPitchObj);
		AubioPitchObj = nullptr;
	}

	// 释放 Onset 对象
	if (AubioOnsetObj)
	{
		del_aubio_onset((aubio_onset_t*)AubioOnsetObj);
		AubioOnsetObj = nullptr;
	}

	// 释放向量缓冲区
	if (InputBuffer)
	{
		del_fvec((fvec_t*)InputBuffer);
		InputBuffer = nullptr;
	}
	if (OutputBuffer)
	{
		del_fvec((fvec_t*)OutputBuffer);
		OutputBuffer = nullptr;
	}
	if (OnsetOutputBuffer)
	{
		del_fvec((fvec_t*)OnsetOutputBuffer);
		OnsetOutputBuffer = nullptr;
	}
}

bool FDreamMusicPlayerAubio::Initialize(int32 SampleRate, int32 BufferSize, int32 HopSize)
{
	// 先清理旧资源，防止内存泄漏
	Cleanup();

	CachedSampleRate = SampleRate;
	CachedBufferSize = BufferSize;
	CachedHopSize = HopSize;

	// 1. 创建数据向量
	InputBuffer = new_fvec(HopSize); // 输入数据长度必须等于 HopSize
	OutputBuffer = new_fvec(1); // Beat 结果
	OnsetOutputBuffer = new_fvec(1); // Onset 结果

	if (!InputBuffer || !OutputBuffer || !OnsetOutputBuffer)
	{
		UE_LOG(LogTemp, Error, TEXT("Aubio: Failed to allocate memory buffers."));
		return false;
	}

	// 2. 创建 Tempo 检测器 ("default" 算法通常表现良好)
	AubioTempoObj = new_aubio_tempo("default", BufferSize, HopSize, SampleRate);

	// 3. 创建 Pitch 检测器 ("yinfft" 是频域算法，准确度较高)
	AubioPitchObj = new_aubio_pitch("yinfft", BufferSize, HopSize, SampleRate);
	if (AubioPitchObj)
	{
		aubio_pitch_set_unit((aubio_pitch_t*)AubioPitchObj, "Hz");
		aubio_pitch_set_tolerance((aubio_pitch_t*)AubioPitchObj, 0.8);
	}

	// 4. 创建 Onset 检测器 ("default" 通常映射为 "hfc" 高频内容算法，适合打击乐)
	AubioOnsetObj = new_aubio_onset("default", BufferSize, HopSize, SampleRate);
	if (AubioOnsetObj)
	{
		aubio_onset_set_threshold((aubio_onset_t*)AubioOnsetObj, OnsetThreshold);
	}

	// 检查核心对象是否创建成功
	if (!AubioTempoObj || !AubioOnsetObj)
	{
		UE_LOG(LogTemp, Error, TEXT("Aubio: Failed to create analysis objects."));
		return false;
	}

	return true;
}

void FDreamMusicPlayerAubio::SetOnsetThreshold(float Threshold)
{
	OnsetThreshold = Threshold;
	if (AubioOnsetObj)
	{
		aubio_onset_set_threshold((aubio_onset_t*)AubioOnsetObj, OnsetThreshold);
	}
}

bool FDreamMusicPlayerAubio::AnalyzeEntireSoundWave(USoundWave* InSoundWave, FDreamMusicAnalysisResult& OutResult)
{
	if (!InSoundWave)
	{
		UE_LOG(LogTemp, Error, TEXT("Aubio: SoundWave is null."));
		return false;
	}

	TArray<uint8> PCMData;
	uint32 SampleRate = 44100;
	uint16 NumChannels = 1;

	// ==================================================================================
	// 阶段 1: 获取原始 PCM 音频数据
	// ==================================================================================

	// 策略 A: 直接内存读取 (通常适用于编辑器模式或 ForceInline 加载模式)
	// 检查 SoundWave 是否已有解压好的数据
	if (InSoundWave->RawPCMData && InSoundWave->RawPCMDataSize > 0)
	{
		PCMData.Append(InSoundWave->RawPCMData, InSoundWave->RawPCMDataSize);
		// 使用 SoundWave.h 中的 Getter 获取平台相关的采样率
		SampleRate = (uint32)InSoundWave->GetSampleRateForCurrentPlatform();
		NumChannels = (uint16)InSoundWave->NumChannels;

		UE_LOG(LogTemp, Log, TEXT("Aubio: Used RawPCMData directly."));
	}
	// 策略 B: 使用解码器解压 (适用于打包后的游戏/Cooked Builds/Compressed Streams)
	else
	{
		// 1. 获取运行时格式名 (如 OGG, BINKA 等)
		FName RuntimeFormat = InSoundWave->GetRuntimeFormat();

		// 2. 创建对应格式的解码器
		ICompressedAudioInfo* Decoder = Audio::CreateSoundAssetDecoder(RuntimeFormat);

		if (!Decoder)
		{
			UE_LOG(LogTemp, Error, TEXT("Aubio: Failed to create decoder for format '%s'. RawPCMData was empty."), *RuntimeFormat.ToString());
			return false;
		}

		// 3. 获取压缩的二进制数据块
		const FPlatformAudioCookOverrides* CompressionOverrides = USoundWave::GetPlatformCompressionOverridesForCurrentPlatform();
		//
		FByteBulkData* CompressedBulkData = InSoundWave->GetCompressedData(RuntimeFormat, CompressionOverrides);

		if (CompressedBulkData && CompressedBulkData->GetBulkDataSize() > 0)
		{
			// 锁定并读取数据
			void* ChunkData = CompressedBulkData->Lock(LOCK_READ_ONLY);
			int32 ChunkSize = CompressedBulkData->GetBulkDataSize();
			FSoundQualityInfo QualityInfo = {0};

			// 4. 解析压缩头信息
			if (Decoder->ReadCompressedInfo((uint8*)ChunkData, ChunkSize, &QualityInfo))
			{
				SampleRate = QualityInfo.SampleRate;
				NumChannels = QualityInfo.NumChannels;

				// 预分配内存 (SampleDataSize 是解压后的大小)
				PCMData.SetNumUninitialized(QualityInfo.SampleDataSize);

				// 5. 解压整个文件到 PCMData
				Decoder->ExpandFile(PCMData.GetData(), &QualityInfo);

				UE_LOG(LogTemp, Log, TEXT("Aubio: Decompressed audio successfully. Size: %d bytes"), PCMData.Num());
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Aubio: Failed to ReadCompressedInfo."));
			}

			CompressedBulkData->Unlock();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Aubio: Compressed data is empty."));
		}

		delete Decoder; // 务必删除解码器
	}

	// 检查数据是否有效
	if (PCMData.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Aubio: Final PCM data is empty. Cannot analyze."));
		return false;
	}

	// ==================================================================================
	// 阶段 2: Aubio 初始化与数据预处理
	// ==================================================================================

	// 保护性检查
	if (SampleRate == 0) SampleRate = 44100;
	if (NumChannels == 0) NumChannels = 1;

	// 参数设置：HopSize 512 在 44.1k 下约为 11ms，精度足够
	int32 BufferSize = 1024;
	int32 HopSize = 512;

	if (!Initialize(SampleRate, BufferSize, HopSize))
	{
		return false;
	}

	// 准备数据转换指针
	// PCMData 是 int16 格式 (Signed 16-bit)
	const int16* SampleDataInt16 = reinterpret_cast<const int16*>(PCMData.GetData());
	int32 TotalSamples = PCMData.Num() / sizeof(int16);
	int32 TotalFrames = TotalSamples / NumChannels;

	// 临时浮点缓冲区
	TArray<float> MonoBuffer;
	MonoBuffer.Reserve(HopSize);

	// 清空结果
	OutResult.BeatTimes.Empty();
	OutResult.OnsetTimes.Empty();
	float SumPitch = 0.0f;
	int32 PitchCount = 0;

	int32 CurrentFrameIndex = 0;

	// ==================================================================================
	// 阶段 3: 循环分帧处理
	// ==================================================================================

	while (CurrentFrameIndex + HopSize < TotalFrames)
	{
		MonoBuffer.Reset();

		// 3.1 提取数据 + 格式转换 + 声道混合
		for (int32 i = 0; i < HopSize; ++i)
		{
			float MixedSample = 0.0f;
			for (int32 Channel = 0; Channel < NumChannels; ++Channel)
			{
				int32 SampleIndex = (CurrentFrameIndex + i) * NumChannels + Channel;

				// 将 int16 (-32768 ~ 32767) 归一化为 float (-1.0 ~ 1.0)
				float SampleVal = (float)SampleDataInt16[SampleIndex] / 32768.0f;
				MixedSample += SampleVal;
			}

			// 混合为单声道
			MixedSample /= (float)NumChannels;
			MonoBuffer.Add(MixedSample);
		}

		// 3.2 调用 Aubio 处理
		bool bIsBeat = ProcessAudio(MonoBuffer);

		// 3.3 收集 Beat 结果
		if (bIsBeat)
		{
			// 获取当前 Beat 时间 (秒)
			float BeatTime = aubio_tempo_get_last_s((aubio_tempo_t*)AubioTempoObj);

			// 简单的防重叠逻辑 (可选)
			if (OutResult.BeatTimes.Num() == 0 || BeatTime > OutResult.BeatTimes.Last() + 0.05f)
			{
				OutResult.BeatTimes.Add(BeatTime);
			}
		}

		// 3.4 收集 Onset 结果 (已经在 ProcessAudio 中计算，但需要在这里获取时间)
		fvec_t* OnsetVec = (fvec_t*)OnsetOutputBuffer;
		if (OnsetVec->data[0] != 0) // data[0] > 0 表示检测到 Onset
		{
			float OnsetTime = aubio_onset_get_last_s((aubio_onset_t*)AubioOnsetObj);
			OutResult.OnsetTimes.Add(OnsetTime);
		}

		// 3.5 收集 Pitch 结果 (过滤静音和低置信度)
		float Pitch = GetPitch();
		float Confidence = GetPitchConfidence();

		// 过滤掉低于 40Hz 的噪音和低置信度结果
		if (Confidence > 0.7f && Pitch > 40.0f)
		{
			SumPitch += Pitch;
			PitchCount++;
		}

		// 步进到下一帧
		CurrentFrameIndex += HopSize;
	}

	// ==================================================================================
	// 阶段 4: 汇总结果
	// ==================================================================================

	OutResult.Bpm = GetBPM(); // 获取最终稳定的 BPM

	if (PitchCount > 0)
	{
		OutResult.AveragePitch = SumPitch / (float)PitchCount;
	}

	UE_LOG(LogTemp, Log, TEXT("Aubio Analysis Done. BPM: %.2f, AvgPitch: %.2f, Beats: %d, Onsets: %d"),
	       OutResult.Bpm, OutResult.AveragePitch, OutResult.BeatTimes.Num(), OutResult.OnsetTimes.Num());

	return true;
}

bool FDreamMusicPlayerAubio::ProcessAudio(const TArray<float>& AudioData)
{
	// 安全检查
	if (!AubioTempoObj || !InputBuffer || AudioData.Num() < CachedHopSize)
	{
		return false;
	}

	fvec_t* InVec = (fvec_t*)InputBuffer;
	fvec_t* OutBeatVec = (fvec_t*)OutputBuffer;
	fvec_t* OutOnsetVec = (fvec_t*)OnsetOutputBuffer;

	// 1. 将 UE float 数组拷贝到 Aubio 的 fvec_t
	for (int32 i = 0; i < CachedHopSize; i++)
	{
		InVec->data[i] = (smpl_t)AudioData[i];
	}

	// 2. 执行节拍 (Beat) 检测
	// 如果检测到 Beat，OutBeatVec->data[0] 会非 0
	aubio_tempo_do((aubio_tempo_t*)AubioTempoObj, InVec, OutBeatVec);

	// 更新 BPM
	if (OutBeatVec->data[0] != 0)
	{
		CurrentBPM = aubio_tempo_get_bpm((aubio_tempo_t*)AubioTempoObj);
	}

	// 3. 执行音高 (Pitch) 检测
	if (AubioPitchObj)
	{
		fvec_t* PitchOut = new_fvec(1);
		aubio_pitch_do((aubio_pitch_t*)AubioPitchObj, InVec, PitchOut);

		CurrentPitch = PitchOut->data[0];
		CurrentConfidence = aubio_pitch_get_confidence((aubio_pitch_t*)AubioPitchObj);

		del_fvec(PitchOut); // 记得释放临时变量
	}

	// 4. 执行起始点 (Onset) 检测
	if (AubioOnsetObj)
	{
		// 如果检测到 Onset，OutOnsetVec->data[0] 会非 0 (通常是 1)
		aubio_onset_do((aubio_onset_t*)AubioOnsetObj, InVec, OutOnsetVec);
	}

	return (OutBeatVec->data[0] != 0);
}

float FDreamMusicPlayerAubio::GetBPM() const
{
	return CurrentBPM;
}

float FDreamMusicPlayerAubio::GetPitch() const
{
	return CurrentPitch;
}

float FDreamMusicPlayerAubio::GetPitchConfidence() const
{
	return CurrentConfidence;
}
