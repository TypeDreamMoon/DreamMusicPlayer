#pragma once

#include "DreamMusicPlayerCommon.h"


struct FDreamMusicPlayerLyricFileParserBase;

namespace FDreamMusicPlayerLyricTools
{
	DREAMMUSICPLAYER_API TArray<FDreamMusicLyric> LoadLyricFromFile(FString FilePath);
	DREAMMUSICPLAYER_API FDreamMusicLyric GetCurrentLyric(FDreamMusicLyricTimestamp CurrentTime, const TArray<FDreamMusicLyric>& Lyrics);
	DREAMMUSICPLAYER_API FDreamMusicLyricTimestamp Conv_TimestampFromFloat(float Time);
	DREAMMUSICPLAYER_API float Conv_FloatFromTimestamp(FDreamMusicLyricTimestamp Timestamp);
	DREAMMUSICPLAYER_API FString GetLyricFilePath(FString FileName);
	DREAMMUSICPLAYER_API TArray<FString> GetLyricFileNames();
}

// Lyrics File Parser
struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricParser
{
	FDreamMusicPlayerLyricParser() = delete;
	FDreamMusicPlayerLyricParser(FString InFilePath, EDreamMusicPlayerLyricParseFileType InFileType, EDreamMusicPlayerLyricParseLineType InLineType);

public:
	FString FilePath;
	FString CachedFileContent;
	TArray<FString> CachedFileLines;
	TMap<FString, FString> MetaData;
	TArray<FDreamMusicLyric> Lyrics;
	EDreamMusicPlayerLyricParseFileType FileType;
	EDreamMusicPlayerLyricParseLineType LineType;
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
	EDreamMusicPlayerLyricParseFileType DetectLRCSubtype() const;

	void ExtractMetadata();
	FString GetMetadata(const FString& Key) const;

	int32 GetLyricCount() const { return Lyrics.Num(); }
	float GetTotalDuration() const;

	bool ValidateTimestamps() const;
	TArray<FString> GetValidationErrors() const;
};
