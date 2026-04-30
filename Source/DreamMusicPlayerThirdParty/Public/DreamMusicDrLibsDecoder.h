// FDrLibsDecoder.h
#pragma once

#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"

// 音频解码结果结构体
struct FDecodedAudioData
{
	// 用于 Aubio 分析的浮点数据 (-1.0 到 1.0)
	TArray<float> PCMDataFloat;
	// 用于 USoundWaveProcedural 播放的 16位整数数据
	TArray<int16> PCMDataInt16;
    
	int32 SampleRate = 44100;
	int32 Channels = 2;
	float Duration = 0.0f;
	bool bSuccess = false;
	FString ErrorMsg;
};

struct DREAMMUSICPLAYERTHIRDPARTY_API FByteRingBuffer
{
	TArray<uint8> Buffer;
	int32 WriteIndex = 0;
	int32 ReadIndex = 0;
	int32 DataAvailable = 0;
	FCriticalSection Mutex;

	void Init(int32 Size) { Buffer.SetNumZeroed(Size); }

	void Write(const uint8* Data, int32 Count)
	{
		FScopeLock Lock(&Mutex);
		// 简单处理：如果这就满了，就丢弃或者这就应该是一个动态扩容的缓冲
		// 这里假设缓冲区足够大 (比如 5MB)
		for (int32 i = 0; i < Count; ++i)
		{
			Buffer[WriteIndex] = Data[i];
			WriteIndex = (WriteIndex + 1) % Buffer.Num();
		}
		DataAvailable = FMath::Min(DataAvailable + Count, Buffer.Num());
	}

	size_t Read(void* OutData, size_t Count)
	{
		FScopeLock Lock(&Mutex);
		size_t ReadCount = FMath::Min((size_t)DataAvailable, Count);
		uint8* OutBytes = (uint8*)OutData;
        
		for (size_t i = 0; i < ReadCount; ++i)
		{
			OutBytes[i] = Buffer[ReadIndex];
			ReadIndex = (ReadIndex + 1) % Buffer.Num();
		}
		DataAvailable -= ReadCount;
		return ReadCount;
	}
};

// 支持的格式
enum class EDrAudioFormat
{
	Unknown,
	Wav,
	Mp3,
	Flac
};

class DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicDrLibsDecoder
{
public:
	static EDrAudioFormat DetectFormat(const uint8* Data, int32 Size);
	static FDecodedAudioData DecodeMemory(const uint8* Data, int32 Size);
    
private:
	static void ConvertFloatToInt16(const TArray<float>& InFloat, TArray<int16>& OutInt16);
};