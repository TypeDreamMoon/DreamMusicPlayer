#pragma once

#include "CoreMinimal.h"

// --- 安全包含 FLAC++ 头文件 ---
// 屏蔽第三方库的常见警告
THIRD_PARTY_INCLUDES_START
#include "FLAC/ordinals.h"       // 定义 FLAC__int32
#include "FLAC/format.h"         // 定义 FLAC__Frame
#include "FLAC/stream_decoder.h" // 定义 WriteStatus
#include "FLAC++/decoder.h"      // 最后才是 C++ 包装
THIRD_PARTY_INCLUDES_END
// ----------------------------

/**
 * 一个简单的结构体，用来返回解码后的数据
 */
struct FFlacResult
{
	TArray<int16> PcmData; // 16-bit PCM 数据
	int32 SampleRate = 0;
	int32 Channels = 0;
	int32 BitDepth = 0;
	bool bSuccess = false;
};

/**
 * 继承 libFLAC++ 的 Decoder::File 类
 */
class FlacDecoderWrapper : public FLAC::Decoder::File
{
public:
	FlacDecoderWrapper();
	virtual ~FlacDecoderWrapper();

	// 对外的主接口
	static FFlacResult Decode(const FString& FilePath);

protected:
	// --- libFLAC++ 的虚函数回调 (Callback) ---
    
	// 当解码器解出一帧音频数据时调用
	virtual ::FLAC__StreamDecoderWriteStatus write_callback(
		const ::FLAC__Frame* frame, 
		const FLAC__int32* const buffer[]
	) override;

	// 当发生错误时调用
	virtual void error_callback(::FLAC__StreamDecoderErrorStatus status) override;

	// 获取元数据（采样率等）
	virtual void metadata_callback(const ::FLAC__StreamMetadata* metadata) override;

private:
	// 内部暂存数据的指针
	TArray<int16>* OutputBufferRef = nullptr;
	FFlacResult CurrentMetadata;
};

class DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicPlayerFlacDecoderWrapper
{
public:
	static FFlacResult Decode(const FString& FilePath);
};