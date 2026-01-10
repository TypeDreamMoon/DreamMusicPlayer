#pragma once

#include "DreamMusicPlayerCommon.h"

struct FDreamMusicPlayerLyricFileParserBase;
struct FDreamMusicLyric;
enum class EDreamMusicPlayerLyricParseLineType : uint8;
enum class EDreamMusicPlayerLyricType : uint8;

// Lyrics File Parser
// FDreamLyricParser已经弃用 请使用DreamLyricParser库
struct UE_DEPRECATED(5.7, "FDreamLyricParser已弃用 请使用DreamLyricParser库 此结构将在下个版本中删除") DREAMMUSICPLAYER_API FDreamLyricParser
{
	FDreamLyricParser() = delete;
	FDreamLyricParser(FString InFilePath, EDreamMusicPlayerLyricType InFileType, EDreamMusicPlayerLyricParseLineType InLineType);

public:
	FString FilePath;
	FString CachedFileContent;
	TArray<FString> CachedFileLines;
	TMap<FString, FString> MetaData;
	TArray<FDreamMusicLyric> Lyrics;
	EDreamMusicPlayerLyricType FileType;
	EDreamMusicPlayerLyricParseLineType LineType;
	// EDreamMusicPlayerLrcLyricType LrcParseMethod = EDreamMusicPlayerLrcLyricType::LineByLine;
	TSharedPtr<FDreamMusicPlayerLyricFileParserBase> Parser;

public:
	void BeginDecodeFile();
	void InitializeParser();

	void ClearCachedLines();
	void ClearLyrics();

	TArray<FDreamMusicLyric> GetLyrics();

	void SortLyricsByTimestamp();
	bool IsValidLyricFile() const;
	FString GetFileExtension() const;

	EDreamMusicPlayerLyricType DetectFileType() const;
	// EDreamMusicPlayerLrcLyricType DetectLRCSubtype() const;

	void ExtractMetadata();
	FString GetMetadata(const FString& Key) const;

	int32 GetLyricCount() const { return Lyrics.Num(); }
	float GetTotalDuration() const;

	bool ValidateTimestamps() const;
	TArray<FString> GetValidationErrors() const;
};
