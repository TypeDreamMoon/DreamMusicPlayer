#pragma once

#include "CoreMinimal.h"
#include "DreamMusicTag.h"

// 解密结果结构体
struct DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicDecryptionResult
{
	bool bSuccess = false;
	FString ErrorMessage;
	TArray<uint8> AudioData;    // 解密后的 PCM 或 压缩流数据
	FString Format;             // flac, mp3, ogg, wav
	FDreamMusicTag Tag;         // 提取出的元数据
	TArray<uint8> CoverData;    // 提取出的封面数据
};

/**
 * 音乐解密器抽象基类
 * 所有具体格式的解密器都应继承此类
 */
class DREAMMUSICPLAYERTHIRDPARTY_API IDreamMusicDecryptor
{
public:
	virtual ~IDreamMusicDecryptor() = default;

	/** 执行解密 */
	virtual FDreamMusicDecryptionResult Decrypt(const FString& FilePath, const TArray<uint8>& FileContent) = 0;

	/** 检查是否支持该文件（通常基于魔数或后缀） */
	virtual bool IsSupported(const FString& Extension, const TArray<uint8>& Header) = 0;

protected:
	// --- 辅助函数：检测音频头 ---
	bool IsFlacHeader(const TArray<uint8>& Data) const
	{
		if (Data.Num() < 4) return false;
		return Data[0] == 0x66 && Data[1] == 0x4C && Data[2] == 0x61 && Data[3] == 0x43; // fLaC
	}

	bool IsMp3Header(const TArray<uint8>& Data) const
	{
		if (Data.Num() < 3) return false;
		return (Data[0] == 0x49 && Data[1] == 0x44 && Data[2] == 0x33) || // ID3
			   (Data[0] == 0xFF && (Data[1] & 0xE0) == 0xE0);             // Sync Frame
	}
    
	bool IsOggHeader(const TArray<uint8>& Data) const
	{
		if (Data.Num() < 4) return false;
		return Data[0] == 'O' && Data[1] == 'g' && Data[2] == 'g' && Data[3] == 'S';
	}
};