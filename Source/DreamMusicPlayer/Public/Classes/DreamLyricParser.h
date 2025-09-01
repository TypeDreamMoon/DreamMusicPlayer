#pragma once

#include "DreamMusicPlayerCommon.h"

struct FDreamMusicPlayerLyricFileParserBase;
struct FDreamMusicLyric;
enum class EDreamMusicPlayerLyricParseLineType : uint8;
enum class EDreamMusicPlayerLyricParseFileType : uint8;

// Lyrics File Parser
struct DREAMMUSICPLAYER_API FDreamLyricParser
{
	FDreamLyricParser() = delete;
	FDreamLyricParser(FString InFilePath, EDreamMusicPlayerLyricParseFileType InFileType, EDreamMusicPlayerLyricParseLineType InLineType, EDreamMusicPlayerLrcLyricType InLrcParseMethod = EDreamMusicPlayerLrcLyricType::None);

public:
	FString FilePath;
	FString CachedFileContent;
	TArray<FString> CachedFileLines;
	TMap<FString, FString> MetaData;
	TArray<FDreamMusicLyric> Lyrics;
	EDreamMusicPlayerLyricParseFileType FileType;
	EDreamMusicPlayerLyricParseLineType LineType;
	EDreamMusicPlayerLrcLyricType LrcParseMethod = EDreamMusicPlayerLrcLyricType::LineByLine;
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

	EDreamMusicPlayerLyricParseFileType DetectFileType() const;
	EDreamMusicPlayerLrcLyricType DetectLRCSubtype() const;

	void ExtractMetadata();
	FString GetMetadata(const FString& Key) const;

	int32 GetLyricCount() const { return Lyrics.Num(); }
	float GetTotalDuration() const;

	bool ValidateTimestamps() const;
	TArray<FString> GetValidationErrors() const;
};
