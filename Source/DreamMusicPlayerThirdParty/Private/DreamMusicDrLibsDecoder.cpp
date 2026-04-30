#include "DreamMusicDrLibsDecoder.h"

THIRD_PARTY_INCLUDES_START

#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"
#define DR_FLAC_IMPLEMENTATION
#include "dr_libs/dr_flac.h"

THIRD_PARTY_INCLUDES_END

EDrAudioFormat FDreamMusicDrLibsDecoder::DetectFormat(const uint8* Data, int32 Size)
{
	if (Size < 4) return EDrAudioFormat::Unknown;

	// WAV Header: "RIFF"
	if (Data[0] == 'R' && Data[1] == 'I' && Data[2] == 'F' && Data[3] == 'F') return EDrAudioFormat::Wav;
	// FLAC Header: "fLaC"
	if (Data[0] == 'f' && Data[1] == 'L' && Data[2] == 'a' && Data[3] == 'C') return EDrAudioFormat::Flac;
	// MP3: ID3 tag or Sync Frame (0xFF, 0xE0)
	if ((Data[0] == 'I' && Data[1] == 'D' && Data[2] == '3') || (Data[0] == 0xFF && (Data[1] & 0xE0) == 0xE0)) return EDrAudioFormat::Mp3;

	return EDrAudioFormat::Unknown;
}

void FDreamMusicDrLibsDecoder::ConvertFloatToInt16(const TArray<float>& InFloat, TArray<int16>& OutInt16)
{
	OutInt16.SetNumUninitialized(InFloat.Num());
	const float* Src = InFloat.GetData();
	int16* Dst = OutInt16.GetData();
	int32 Num = InFloat.Num();

	for (int32 i = 0; i < Num; ++i)
	{
		// 简单的硬裁剪防止爆音
		float Val = Src[i] * 32767.0f;
		if (Val > 32767.0f) Val = 32767.0f;
		else if (Val < -32768.0f) Val = -32768.0f;
		Dst[i] = (int16)Val;
	}
}

FDecodedAudioData FDreamMusicDrLibsDecoder::DecodeMemory(const uint8* Data, int32 Size)
{
	FDecodedAudioData Result;
	EDrAudioFormat Format = DetectFormat(Data, Size);

	float* RawFloatData = nullptr;
	uint64 TotalFrames = 0;
	unsigned int Channels = 0;
	unsigned int SampleRate = 0;

	if (Format == EDrAudioFormat::Wav)
	{
		RawFloatData = drwav_open_memory_and_read_pcm_frames_f32(Data, Size, &Channels, &SampleRate, &TotalFrames, NULL);
	}
	else if (Format == EDrAudioFormat::Mp3)
	{
		drmp3_config Config;
		RawFloatData = drmp3_open_memory_and_read_pcm_frames_f32(Data, Size, &Config, &TotalFrames, NULL);
		Channels = Config.channels;
		SampleRate = Config.sampleRate;
	}
	else if (Format == EDrAudioFormat::Flac)
	{
		RawFloatData = drflac_open_memory_and_read_pcm_frames_f32(Data, Size, &Channels, &SampleRate, &TotalFrames, NULL);
	}
	else
	{
		Result.ErrorMsg = TEXT("Unknown or Unsupported Audio Format");
		return Result;
	}

	if (RawFloatData && TotalFrames > 0 && Channels > 0)
	{
		Result.Channels = Channels;
		Result.SampleRate = SampleRate;
		int32 TotalSamples = TotalFrames * Channels;
		Result.Duration = (float)TotalFrames / (float)SampleRate;

		// 1. 填充 Float 数据 (直接用于 Aubio)
		Result.PCMDataFloat.Append(RawFloatData, TotalSamples);

		// 2. 转换并填充 Int16 数据 (用于 SoundWave)
		ConvertFloatToInt16(Result.PCMDataFloat, Result.PCMDataInt16);

		// 释放 dr_libs 内存
		if (Format == EDrAudioFormat::Wav) drwav_free(RawFloatData, NULL);
		else if (Format == EDrAudioFormat::Mp3) drmp3_free(RawFloatData, NULL);
		else if (Format == EDrAudioFormat::Flac) drflac_free(RawFloatData, NULL);

		Result.bSuccess = true;
	}
	else
	{
		Result.ErrorMsg = TEXT("Failed to decode audio frames via dr_libs");
	}

	return Result;
}
