#pragma once

#include "CoreMinimal.h"
#include "DreamMusicDecryptorBase.h"

// --- 1. NCM 解密器 (网易云) ---
class DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicNcmDecryptor : public IDreamMusicDecryptor
{
public:
	virtual FDreamMusicDecryptionResult Decrypt(const FString& FilePath, const TArray<uint8>& FileContent) override;
	virtual bool IsSupported(const FString& Extension, const TArray<uint8>& Header) override;

private:
	void Aes128DecryptEcb(const uint8* InKey, uint8* InOutData, int32 DataSize);
	void GenerateRc4KeyStream(const TArray<uint8>& KeyData, TArray<uint8>& OutKeyStream);
	void ParseMetadata(const TArray<uint8>& InJsonData, FDreamMusicTag& OutTag);
};

// --- 2. QMC 解密器 (QQ音乐: QMCv1, QMCv2, MFLAC, MGG) ---
class FDreamMusicQmcDecryptor : public IDreamMusicDecryptor
{
public:
	virtual FDreamMusicDecryptionResult Decrypt(const FString& FilePath, const TArray<uint8>& FileContent) override;
	virtual bool IsSupported(const FString& Extension, const TArray<uint8>& Header) override;

private:
	void ApplyStaticCipher(TArray<uint8>& Data);
	void ApplyMapCipher(TArray<uint8>& Data); // 简化版占位
};

// --- 3. KGM 解密器 (酷狗) ---
class FDreamMusicKgmDecryptor : public IDreamMusicDecryptor
{
public:
	virtual FDreamMusicDecryptionResult Decrypt(const FString& FilePath, const TArray<uint8>& FileContent) override;
	virtual bool IsSupported(const FString& Extension, const TArray<uint8>& Header) override;
};

// --- 4. XM 解密器 (虾米) ---
class FDreamMusicXmDecryptor : public IDreamMusicDecryptor
{
public:
	virtual FDreamMusicDecryptionResult Decrypt(const FString& FilePath, const TArray<uint8>& FileContent) override;
	virtual bool IsSupported(const FString& Extension, const TArray<uint8>& Header) override;
};

// --- 5. KW 解密器 (酷我) ---
class FDreamMusicKwDecryptor : public IDreamMusicDecryptor
{
public:
	virtual FDreamMusicDecryptionResult Decrypt(const FString& FilePath, const TArray<uint8>& FileContent) override;
	virtual bool IsSupported(const FString& Extension, const TArray<uint8>& Header) override;
};

// --- 解密器工厂 ---
class DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicDecryptorFactory
{
public:
	/** 自动匹配并解密 */
	static FDreamMusicDecryptionResult DecryptFile(const FString& InFilePath);
};
