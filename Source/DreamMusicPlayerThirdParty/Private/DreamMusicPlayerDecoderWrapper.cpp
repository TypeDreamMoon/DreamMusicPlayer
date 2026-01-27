#include "DreamMusicPlayerDecoderWrapper.h"
#include "Misc/Paths.h"

FlacDecoderWrapper::FlacDecoderWrapper() : FLAC::Decoder::File()
{
}

FlacDecoderWrapper::~FlacDecoderWrapper()
{
}

FFlacResult FlacDecoderWrapper::Decode(const FString& FilePath)
{
	FFlacResult Result;

	// 1. 检查文件是否存在
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("[FlacDecoder] File not found: %s"), *FilePath);
		return Result;
	}

	// 2. 实例化解码器
	FlacDecoderWrapper Decoder;

	// 把结果引用传进去，方便回调函数填充
	Decoder.OutputBufferRef = &Result.PcmData;

	// 3. 初始化 (路径转 UTF-8)
	// 注意：Windows 路径包含中文时，TCHAR_TO_UTF8 非常关键
	FLAC__StreamDecoderInitStatus InitStatus = Decoder.init(TCHAR_TO_UTF8(*FilePath));

	if (InitStatus != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		UE_LOG(LogTemp, Error, TEXT("[FlacDecoder] Init failed"));
		return Result;
	}

	// 4. 执行解码 (阻塞直到结束)
	bool bProcessResult = Decoder.process_until_end_of_stream();

	// 5. 填充结果
	Result.bSuccess = bProcessResult;
	Result.Channels = Decoder.CurrentMetadata.Channels;
	Result.SampleRate = Decoder.CurrentMetadata.SampleRate;
	Result.BitDepth = Decoder.CurrentMetadata.BitDepth;

	// 如果 metadata_callback 没触发（无头文件），尝试从最后已知的状态补全
	if (Result.SampleRate == 0)
	{
		// 这种情况很少见，但在某些 raw flac 流中可能发生
		UE_LOG(LogTemp, Warning, TEXT("[FlacDecoder] No metadata found, wrapper might be incomplete."));
	}

	return Result;
}

// --- 回调函数实现 ---

::FLAC__StreamDecoderWriteStatus FlacDecoderWrapper::write_callback(const ::FLAC__Frame* frame, const FLAC__int32* const buffer[])
{
	if (!OutputBufferRef) return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

	// 获取当前块的信息
	const uint32 BlockSize = frame->header.blocksize;
	const uint32 Channels = frame->header.channels;
	const uint32 BitsPerSample = frame->header.bits_per_sample;

	// 更新元数据（如果是第一帧，或者流中间参数变了）
	if (CurrentMetadata.SampleRate == 0)
	{
		CurrentMetadata.SampleRate = frame->header.sample_rate;
		CurrentMetadata.Channels = Channels;
		CurrentMetadata.BitDepth = BitsPerSample;
	}

	// 预分配内存以优化性能 (可选)
	// OutputBufferRef->Reserve(OutputBufferRef->Num() + BlockSize * Channels);

	/* 重要逻辑：FLAC输出是非交错(Non-interleaved)的二维数组 buffer[channel][sample]
	   Unreal 音频通常需要交错(Interleaved)的一维数组 L,R,L,R...
	*/
	for (uint32 i = 0; i < BlockSize; i++)
	{
		for (uint32 Channel = 0; Channel < Channels; Channel++)
		{
			FLAC__int32 Sample32 = buffer[Channel][i];
			int16 Sample16 = 0;

			// 位深转换逻辑
			if (BitsPerSample == 16)
			{
				Sample16 = static_cast<int16>(Sample32);
			}
			else if (BitsPerSample == 24)
			{
				// 24bit 转 16bit：简单的做法是右移 8 位
				Sample16 = static_cast<int16>(Sample32 >> 8);
			}
			else if (BitsPerSample == 8)
			{
				// 8bit 转 16bit：左移 8 位
				Sample16 = static_cast<int16>(Sample32 << 8);
			}
			else
			{
				// 默认回退
				Sample16 = static_cast<int16>(Sample32);
			}

			OutputBufferRef->Add(Sample16);
		}
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacDecoderWrapper::metadata_callback(const ::FLAC__StreamMetadata* metadata)
{
	// 这里可以获取总采样数、艺术家信息等
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
	{
		CurrentMetadata.SampleRate = metadata->data.stream_info.sample_rate;
		CurrentMetadata.Channels = metadata->data.stream_info.channels;
		CurrentMetadata.BitDepth = metadata->data.stream_info.bits_per_sample;
		// TotalSamples = metadata->data.stream_info.total_samples; 
	}
}

FFlacResult FDreamMusicPlayerFlacDecoderWrapper::Decode(const FString& FilePath)
{
	return FlacDecoderWrapper::Decode(FilePath);
}

void FlacDecoderWrapper::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
	// 将 FLAC 错误码转为字符串并打印
	UE_LOG(LogTemp, Error, TEXT("[FlacDecoder] Error Callback: %d"), (int32)status);
}
