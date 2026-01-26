#pragma once

#include "CoreMinimal.h"
#include "DreamMusicFileType.h"
#include "Engine/Texture2D.h"
#include "DreamMusicTag.generated.h"

enum class EDreamMusicPlayerTagLibFileFormat : uint8;

/**
 * 纯数据结构：用于在 UI 和逻辑之间传递音乐元数据
 */
USTRUCT(BlueprintType)
struct DREAMMUSICPLAYER_API FDreamMusicTag
{
	GENERATED_BODY()

public:
	// --- 基础信息 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Info")
	FString Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Info")
	FString Artist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Info")
	FString Album;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Info")
	FString Genre;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Info")
	FString Comment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Info")
	int32 Year;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Info")
	int32 Track;

	// --- 音频属性 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Properties")
	int32 Duration; // 秒

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Properties")
	int32 Bitrate; // kbps

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Properties")
	int32 SampleRate; // Hz

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Properties")
	int32 Channels;

	// --- 文件信息 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File Info")
	FString FilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "File Info")
	EDreamMusicPlayerTagLibFileFormat FileType;

	// --- 封面 ---
	/** * 封面贴图 
	 * 注意：如果在非主线程构建此结构体，此字段必须为空，否则会导致 Crash
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	UTexture2D* CoverArt;

	// --- 构造函数 ---
	FDreamMusicTag()
		: Year(0), Track(0), Duration(0), Bitrate(0), SampleRate(0), Channels(0),
		  FileType(EDreamMusicPlayerTagLibFileFormat::Unknown), CoverArt(nullptr)
	{
	}

	// --- 辅助：打印调试信息 ---
	FString ToString() const
	{
		return FString::Printf(TEXT("[%s] %s - %s (%d:%02d)"),
		                       *UEnum::GetDisplayValueAsText(FileType).ToString(), *Artist, *Title, Duration / 60, Duration % 60);
	}


	bool operator==(const FDreamMusicTag& Other) const;
	bool IsValid() const;
};

inline bool FDreamMusicTag::operator==(const FDreamMusicTag& Other) const
{
	return Title == Other.Title && Artist == Other.Artist && Album == Other.Album && Genre == Other.Genre && Comment == Other.Comment && Year == Other.Year && Track == Other.Track && Duration == Other.Duration && Bitrate == Other.Bitrate && SampleRate == Other.SampleRate && Channels == Other.Channels && FilePath == Other.FilePath && FileType == Other.FileType && CoverArt == Other.CoverArt;
}

inline bool FDreamMusicTag::IsValid() const
{
	return !Title.IsEmpty() || !Artist.IsEmpty() || !Album.IsEmpty() || !Genre.IsEmpty() || !Comment.IsEmpty() || Year > 0 || Track > 0 || Duration > 0 || Bitrate > 0 || SampleRate > 0 || Channels > 0 || !FilePath.IsEmpty() || FileType != EDreamMusicPlayerTagLibFileFormat::Unknown;
}
