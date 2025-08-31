#pragma once

#include "DreamMusicPlayerCommon.h"

struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParserBase
{
public:
	FDreamMusicPlayerLyricFileParserBase() = delete;

	FDreamMusicPlayerLyricFileParserBase(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLyricParseLineType InLineType)
		: FileContent(InFileContent), Lines(InLines), LineType(InLineType)
	{
	}

	virtual ~FDreamMusicPlayerLyricFileParserBase() = default;

protected:
	FString FileContent;
	TArray<FString> Lines;
	TArray<FDreamMusicLyric> ParsedLyrics;
	EDreamMusicPlayerLyricParseLineType LineType;

public:
	virtual void Parse();
	virtual void ProcessText(FDreamMusicLyric& Lyric);

	// Get parsed lyrics
	const TArray<FDreamMusicLyric>& GetParsedLyrics() const { return ParsedLyrics; }

	// Clear parsed data
	virtual void ClearParsedData() { ParsedLyrics.Empty(); }
};

struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParser_SRT : public FDreamMusicPlayerLyricFileParserBase
{
public:
	FDreamMusicPlayerLyricFileParser_SRT(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLyricParseLineType InLineType)
		: FDreamMusicPlayerLyricFileParserBase(InFileContent, InLines, InLineType)
	{
	}

public:
	virtual void Parse() override;
	virtual void ProcessText(FDreamMusicLyric& Lyric) override;

protected:
	// Parse SRT timestamp line (e.g., "00:00:48,710 --> 00:00:58,770")
	bool ParseSRTTimestamp(const FString& TimestampLine, FDreamMusicLyric& OutLyric);

	// Parse individual SRT time format (HH:MM:SS,mmm)
	FDreamMusicLyricTimestamp ParseSRTTime(const FString& TimeString);

	// // Clean SRT content from formatting tags
	// FString CleanSRTContent(const FString& RawContent);
};

struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParser_LRC : public FDreamMusicPlayerLyricFileParserBase
{
public:
	FDreamMusicPlayerLyricFileParser_LRC(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLrcLyricType InParseMethod, EDreamMusicPlayerLyricParseLineType InLineType)
		: FDreamMusicPlayerLyricFileParserBase(InFileContent, InLines, InLineType), ParseMethod(InParseMethod)
	{
	}

protected:
	EDreamMusicPlayerLrcLyricType ParseMethod;

public:
	virtual void Parse() override;
	virtual void ProcessText(FDreamMusicLyric& Lyric) override;
};

struct DREAMMUSICPLAYER_API FDreamMusicPlayerLyricFileParser_ASS : public FDreamMusicPlayerLyricFileParserBase
{
public:
	FDreamMusicPlayerLyricFileParser_ASS(const FString& InFileContent, const TArray<FString>& InLines, EDreamMusicPlayerLyricParseLineType InLineType)
		: FDreamMusicPlayerLyricFileParserBase(InFileContent, InLines, InLineType)
	{
	}

public:
	virtual void Parse() override;
	virtual void ProcessText(FDreamMusicLyric& Lyric) override;
};
