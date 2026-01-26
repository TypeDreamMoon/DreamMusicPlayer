// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DreamMusicFileType.h"
#include "DreamMusicTag.h"
#include "taglib/tag.h"

#include "Engine/Texture2D.h"

namespace TagLib
{
	class File;
	class FileRef;
	class Tag;
	class AudioProperties;
}


USTRUCT(BlueprintType)
struct DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicPlayerTagLibFileFormat
{
	GENERATED_BODY()

	FDreamMusicPlayerTagLibFileFormat()
		: FileFormat(EDreamMusicPlayerTagLibFileFormat::Unknown), File(nullptr)
	{
	}

	FDreamMusicPlayerTagLibFileFormat(EDreamMusicPlayerTagLibFileFormat Format, TagLib::File* InFile)
		: FileFormat(Format), File(InFile)
	{
	}

	// 静态辅助函数
	static FDreamMusicPlayerTagLibFileFormat Unknown()
	{
		return FDreamMusicPlayerTagLibFileFormat(EDreamMusicPlayerTagLibFileFormat::Unknown, nullptr);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDreamMusicPlayerTagLibFileFormat FileFormat;

	// 注意：这个原始指针由 TagLib::FileRef 管理，不要在外部 delete 它
	TagLib::File* File;
};

/**
 * 
 */
struct DREAMMUSICPLAYERTHIRDPARTY_API FDreamMusicPlayerTagLib
{
public:
	FDreamMusicPlayerTagLib(const FString& InFilePath);
	~FDreamMusicPlayerTagLib();

	// 核心判定
	FDreamMusicPlayerTagLibFileFormat DetectFileFormat() const;

	bool IsValid() const;
	bool Save(); // 新增：保存修改

	// --- 基础标签 (Basic Tags) ---
	FString GetTitle() const;
	FString GetArtist() const;
	FString GetAlbum() const;
	FString GetGenre() const;
	FString GetComment() const;
	int GetYear() const;
	int GetTrack() const;

	void SetTitle(const FString& InTitle);
	void SetArtist(const FString& InArtist);
	void SetAlbum(const FString& InAlbum);
	void SetGenre(const FString& InGenre);
	void SetComment(const FString& InComment);
	void SetYear(int InYear);
	void SetTrack(int InTrack);

	// --- 音频属性 (Audio Properties) [新增] ---
	int GetDuration() const; // 秒
	int GetBitrate() const; // kbps
	int GetSampleRate() const; // Hz
	int GetChannels() const; // 声道数

	// --- 高级功能 ---
	// 获取所有扩展属性
	TMap<FString, FString> GetProperties() const; // 修改为 FString 值更通用

	// 获取封面图片数据 (返回二进制数据，可用于创建 Texture2D)
	bool GetCoverArt(TArray<uint8>& OutData, FString& OutMimeType) const;

	/**
	 * 直接获取封面为 Texture2D
	 */
	UTexture2D* GetCoverArtTexture() const;
	
	/**
	 * 将所有解析到的数据打包成 UE 结构体
	 * @param bLoadCoverTexture 是否加载封面图？(必须在 GameThread 调用)
	 */
	FDreamMusicTag GetTagData(bool bLoadCoverTexture = true);

private:
	TagLib::FileRef* FileRef;

	// 缓存指针，避免每次通过 FileRef 查找
	TagLib::Tag* Tag;
	TagLib::AudioProperties* AudioProperties;
};
