#pragma once

#include "DreamMusicPlayerCommon.h"

/**
 * @brief Utility structure for processing grouped lyric lines
 * 
 * Handles the common logic for processing lines grouped by timestamp
 * across different lyric formats (LineByLine, WordByWord, ESLyric)
 */
struct FDreamLyricGroupProcessor
{
public:
	/**
	 * @brief Constructor
	 * 
	 * @param InParseMethod The lyric parsing method (LineByLine, WordByWord, ESLyric)
	 * @param InLineType The line type configuration for content assignment
	 */
	FDreamLyricGroupProcessor(EDreamMusicPlayerLrcLyricType InParseMethod, EDreamMusicPlayerLyricParseLineType InLineType)
		: ParseMethod(InParseMethod), LineType(InLineType)
	{
	}

	/**
	 * @brief Process a group of lines with the same timestamp
	 * 
	 * @param LinesInGroup Array of lines that share the same timestamp
	 * @param OutLyric The output lyric object to populate
	 */
	void ProcessGroup(const TArray<FString>& LinesInGroup, FDreamMusicLyric& OutLyric);

public:
	EDreamMusicPlayerLrcLyricType ParseMethod;
	EDreamMusicPlayerLyricParseLineType LineType;

	/**
	 * @brief Get expected number of lines based on LineType
	 */
	int32 GetExpectedLineCount() const;

	/**
	 * @brief Assign content to lyric fields based on LineType and line order
	 */
	void AssignContentByLineType(FDreamMusicLyric& Lyric, const TArray<FString>& LinesInGroup, int32 ExpectedLines);

	/**
	 * @brief Process a single line and assign to specific field
	 */
	void ProcessLineToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField);

	/**
	 * @brief Extract timestamp-content pairs from a line
	 */
	bool ExtractTimestampContentPairs(const FString& Line, TArray<FDreamMusicLyricTimestamp>& OutTimestamps, TArray<FString>& OutContents, const FString& RegexPattern);

	/**
	 * @brief Build word timings from timestamps and content
	 */
	FString BuildWordsFromTimestamps(const TArray<FDreamMusicLyricTimestamp>& Timestamps, const TArray<FString>& Contents, TArray<FDreamMusicLyricWord>& OutWords);

	/**
	 * @brief Calculate end timestamp for a character
	 */
	FDreamMusicLyricTimestamp CalculateEndTimestamp(const FDreamMusicLyricTimestamp& StartTimestamp, int32 CharIndex, const FString& WordContent, int32 SegmentIndex, const TArray<FDreamMusicLyricTimestamp>& AllTimestamps);

	/**
	 * @brief Create timestamp from total milliseconds
	 */
	FDreamMusicLyricTimestamp CreateTimestampFromMs(int32 TotalMs);

	/**
	 * @brief Assign content and word timings to appropriate field
	 */
	void AssignToField(FDreamMusicLyric& Lyric, const FString& Content, const TArray<FDreamMusicLyricWord>& Words, const FString& TargetField);

	/**
	 * @brief Extract content from LRC line (removes timestamp)
	 */
	FString ExtractContentFromLine(const FString& Line);

	void ProcessWordByWordToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField);
	void ProcessESLyricToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField);
	void ProcessLineByLineToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField);
};