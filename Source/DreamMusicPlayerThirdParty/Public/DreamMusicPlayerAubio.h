// Copyright Dream Moon. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundWave.h"

/**
 * 音乐分析结果结构体
 * 包含 BPM、音高、节拍点和起始点信息
 */
struct FDreamMusicAnalysisResult
{
	// 估算的每分钟节拍数 (Beats Per Minute)
	float Bpm = 0.0f;

	// 平均音高 (Hz)，用于判断整体调性
	float AveragePitch = 0.0f;

	// 节拍时间点数组 (秒) - 对应更有规律的节奏 (动、次、打、次)
	TArray<float> BeatTimes;

	// 起始时间点数组 (秒) - 对应所有发声瞬间 (Note On)，如钢琴按下、鼓点敲击
	// 通常用于生成音游谱面 (Notes)
	TArray<float> OnsetTimes;
};

/**
 * Aubio 库的封装类
 * 负责音频信号处理、BPM 分析、音高检测和 Onset 检测
 */
class DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicPlayerAubio
{
public:
	FDreamMusicPlayerAubio();
	~FDreamMusicPlayerAubio();

	/**
	 * [核心功能] 分析整个 SoundWave 资源
	 * 自动处理音频解压、声道混合和分帧处理
	 * @param PCMData  输入的PCM 数据
	 * @param OutResult    输出的分析结果
	 * @param SampleRate   采样率 (如 44100)
	 * @param NumChannels 声道数 (如 1)
	 * @return             是否分析成功
	 * * @note 这是一个耗时操作，建议在异步线程 (AsyncTask) 中调用，以免阻塞游戏主线程。
	 */
	bool AnalyzeEntireSoundWave(const TArray<uint8>& PCMData, int SampleRate, int NumChannels, FDreamMusicAnalysisResult& OutResult);

	/**
	 * 设置 Onset (起始点) 检测的灵敏度阈值
	 * @param Threshold 默认约 0.3。数值越小越灵敏（可能误检），数值越大越严格（可能漏检）。
	 */
	void SetOnsetThreshold(float Threshold);

	// bool AnalyzeRawAudio(const TArray<float>& RawPCMData, int32 NumChannels, int32 SampleRate, FDreamMusicAnalysisResult& OutResult);

	/**
	 * @warning 该函数需要在音频播放前调用
	 * @param InSoundWave  输入的 SoundWave 资源
	 * @param OutPCMData  输出的 PCM 数据
	 * @param OutSampleRate    采样率
	 * @param OutNumChannels    声道数
	 * @return 是否成功解码
	 */
	bool DecodeSoundWave(USoundWave* InSoundWave, TArray<uint8>& OutPCMData, int& OutSampleRate, int& OutNumChannels);
private:
	/**
	 * 初始化 Aubio 内部对象
	 * @param SampleRate 采样率 (如 44100)
	 * @param BufferSize 处理窗口大小 (通常 1024)
	 * @param HopSize    步长 (通常 512)，决定了时间精度
	 */
	bool Initialize(int32 SampleRate, int32 BufferSize, int32 HopSize);

	/**
	 * 清理并释放所有 Aubio 资源
	 */
	void Cleanup();

	/**
	 * 处理单帧音频数据
	 * @param AudioData 单声道的浮点音频数据，长度必须等于 HopSize
	 * @return 如果当前帧检测到 Beat，返回 true
	 */
	bool ProcessAudio(const TArray<float>& AudioData);

	// --- 实时数据获取 ---
	float GetBPM() const;
	float GetPitch() const;
	float GetPitchConfidence() const; // 音高检测的可信度 (0.0 - 1.0)

private:
	// --- Aubio 不透明指针 (void*) ---
	// 使用 void* 避免在头文件中包含 aubio.h，防止符号污染 UE 命名空间
	void* AubioTempoObj; // 节拍检测器 (aubio_tempo_t)
	void* AubioPitchObj; // 音高检测器 (aubio_pitch_t)
	void* AubioOnsetObj; // 起始点检测器 (aubio_onset_t)

	// --- Aubio 数据缓冲区 ---
	void* InputBuffer; // 输入向量 (fvec_t)
	void* OutputBuffer; // Beat 输出向量
	void* OnsetOutputBuffer; // Onset 输出向量

	// --- 缓存参数 ---
	int32 CachedSampleRate;
	int32 CachedBufferSize;
	int32 CachedHopSize;

	// --- 当前帧结果 ---
	float CurrentBPM;
	float CurrentPitch;
	float CurrentConfidence;

	// --- 配置参数 ---
	float OnsetThreshold;
};
