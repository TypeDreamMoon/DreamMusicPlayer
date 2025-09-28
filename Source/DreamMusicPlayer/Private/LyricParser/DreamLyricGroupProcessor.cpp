#include "LyricParser/DreamLyricGroupProcessor.h"

void FDreamLyricGroupProcessor::ProcessGroup(const TArray<FString>& LinesInGroup, FDreamMusicLyric& OutLyric)
{
	if (LinesInGroup.Num() == 0)
		return;

	int32 ExpectedLines = GetExpectedLineCount();
	int32 ActualLines = LinesInGroup.Num();

	if (ActualLines >= ExpectedLines)
	{
		// We have enough lines, assign according to LineType
		AssignContentByLineType(OutLyric, LinesInGroup, ExpectedLines);
	}
	else if (ActualLines == 2 && ExpectedLines == 3)
	{
		// Fallback to 2-line mode: first line = lyrics, second line = translation
		ProcessLineToField(OutLyric, LinesInGroup[0], TEXT("Content"));
		ProcessLineToField(OutLyric, LinesInGroup[1], TEXT("Translate"));
	}
	else if (ActualLines == 1)
	{
		// Fallback to 1-line mode: only lyrics, no translation
		ProcessLineToField(OutLyric, LinesInGroup[0], TEXT("Content"));
	}
}

int32 FDreamLyricGroupProcessor::GetExpectedLineCount() const
{
	switch (LineType)
	{
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
		return 3;
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
		return 2;
	case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
	default:
		return 1;
	}
}

void FDreamLyricGroupProcessor::AssignContentByLineType(FDreamMusicLyric& Lyric, const TArray<FString>& LinesInGroup, int32 ExpectedLines)
{
	switch (LineType)
	{
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Romanization"));
		if (LinesInGroup.Num() >= 2) ProcessLineToField(Lyric, LinesInGroup[1], TEXT("Content"));
		if (LinesInGroup.Num() >= 3) ProcessLineToField(Lyric, LinesInGroup[2], TEXT("Translate"));
		break;
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Content"));
		if (LinesInGroup.Num() >= 2) ProcessLineToField(Lyric, LinesInGroup[1], TEXT("Romanization"));
		if (LinesInGroup.Num() >= 3) ProcessLineToField(Lyric, LinesInGroup[2], TEXT("Translate"));
		break;
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Translate"));
		if (LinesInGroup.Num() >= 2) ProcessLineToField(Lyric, LinesInGroup[1], TEXT("Content"));
		if (LinesInGroup.Num() >= 3) ProcessLineToField(Lyric, LinesInGroup[2], TEXT("Romanization"));
		break;
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Romanization"));
		if (LinesInGroup.Num() >= 2) ProcessLineToField(Lyric, LinesInGroup[1], TEXT("Content"));
		break;
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Content"));
		if (LinesInGroup.Num() >= 2) ProcessLineToField(Lyric, LinesInGroup[1], TEXT("Romanization"));
		break;
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Translate"));
		if (LinesInGroup.Num() >= 2) ProcessLineToField(Lyric, LinesInGroup[1], TEXT("Content"));
		break;
	case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Content"));
		if (LinesInGroup.Num() >= 2) ProcessLineToField(Lyric, LinesInGroup[1], TEXT("Translate"));
		break;
	case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
	default:
		if (LinesInGroup.Num() >= 1) ProcessLineToField(Lyric, LinesInGroup[0], TEXT("Content"));
		break;
	}
}

void FDreamLyricGroupProcessor::ProcessLineToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField)
{
	switch (ParseMethod)
	{
	case EDreamMusicPlayerLrcLyricType::WordByWord:
		ProcessWordByWordToField(Lyric, Line, TargetField);
		break;
	case EDreamMusicPlayerLrcLyricType::ESLyric:
		ProcessESLyricToField(Lyric, Line, TargetField);
		break;
	case EDreamMusicPlayerLrcLyricType::LineByLine:
	default:
		ProcessLineByLineToField(Lyric, Line, TargetField);
		break;
	}
}

void FDreamLyricGroupProcessor::ProcessWordByWordToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField)
{
	TArray<FDreamMusicLyricTimestamp> Timestamps;
	TArray<FString> Contents;

	if (!ExtractTimestampContentPairs(Line, Timestamps, Contents, TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\]([^\\[]*)")))
	{
		ProcessLineByLineToField(Lyric, Line, TargetField);
		return;
	}

	TArray<FDreamMusicLyricWord> Words;
	// Use the new word-segment based approach instead of character-by-character
	FString FullContent = BuildWordTimingsFromSegments(Timestamps, Contents, Words);
	AssignToField(Lyric, FullContent, Words, TargetField);
}

void FDreamLyricGroupProcessor::ProcessESLyricToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField)
{
	TArray<FDreamMusicLyricTimestamp> Timestamps;
	TArray<FString> Contents;

	if (!ExtractTimestampContentPairs(Line, Timestamps, Contents, TEXT("<(\\d{2}):(\\d{2})[.:](\\d{2,3})>([^<]*)")))
	{
		ProcessLineByLineToField(Lyric, Line, TargetField);
		return;
	}

	TArray<FDreamMusicLyricWord> Words;
	FString FullContent = BuildWordTimingsFromSegments(Timestamps, Contents, Words);
	AssignToField(Lyric, FullContent, Words, TargetField);
}

void FDreamLyricGroupProcessor::ProcessLineByLineToField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField)
{
	FString ExtractedContent = ExtractContentFromLine(Line);
	TArray<FDreamMusicLyricWord> EmptyWords; // No word timings for LineByLine mode
	AssignToField(Lyric, ExtractedContent, EmptyWords, TargetField);
}

bool FDreamLyricGroupProcessor::ExtractTimestampContentPairs(const FString& Line, TArray<FDreamMusicLyricTimestamp>& OutTimestamps, TArray<FString>& OutContents, const FString& RegexPattern)
{
	FRegexPattern Pattern(RegexPattern);
	FRegexMatcher Matcher(Pattern, Line);

	OutTimestamps.Empty();
	OutContents.Empty();

	while (Matcher.FindNext())
	{
		int32 Minutes = FCString::Atoi(*Matcher.GetCaptureGroup(1));
		int32 Seconds = FCString::Atoi(*Matcher.GetCaptureGroup(2));
		FString MillisecondsStr = Matcher.GetCaptureGroup(3);
		FString WordContent = Matcher.GetCaptureGroup(4);

		int32 Milliseconds = (MillisecondsStr.Len() == 2) ? FCString::Atoi(*MillisecondsStr) * 10 : FCString::Atoi(*MillisecondsStr);
		FDreamMusicLyricTimestamp Timestamp(Minutes, Seconds, Milliseconds);

		OutTimestamps.Add(Timestamp);
		OutContents.Add(WordContent);
	}

	return OutTimestamps.Num() > 0;
}

// NEW METHOD: Build word timings from word segments (similar to ASS parsing)
FString FDreamLyricGroupProcessor::BuildWordTimingsFromSegments(const TArray<FDreamMusicLyricTimestamp>& Timestamps, const TArray<FString>& Contents, TArray<FDreamMusicLyricWord>& OutWords)
{
	OutWords.Empty();
	FString FullContent;

	for (int32 i = 0; i < Timestamps.Num() && i < Contents.Num(); i++)
	{
		FDreamMusicLyricTimestamp StartTimestamp = Timestamps[i];
		FString WordContent = Contents[i];

		// Skip empty content
		if (WordContent.IsEmpty())
			continue;

		// Calculate end timestamp for this word segment
		FDreamMusicLyricTimestamp EndTimestamp;
		if (i + 1 < Timestamps.Num())
		{
			// Use next timestamp as end time
			EndTimestamp = Timestamps[i + 1];
		}
		else
		{
			// For last segment, add default duration (500ms)
			int32 EndMs = StartTimestamp.ToMilliseconds() + 500;
			EndTimestamp = CreateTimestampFromMs(EndMs);
		}

		// Create word entry for the entire segment content
		// This preserves the complete word/phrase from each timestamp segment
		FDreamMusicLyricWord Word(StartTimestamp, EndTimestamp, WordContent);
		OutWords.Add(Word);
		FullContent += WordContent;
	}

	return FullContent;
}

// DEPRECATED: Old character-by-character method (keeping for compatibility)
FString FDreamLyricGroupProcessor::BuildWordsFromTimestamps(const TArray<FDreamMusicLyricTimestamp>& Timestamps, const TArray<FString>& Contents, TArray<FDreamMusicLyricWord>& OutWords)
{
	OutWords.Empty();
	FString FullContent;

	for (int32 i = 0; i < Timestamps.Num(); i++)
	{
		FDreamMusicLyricTimestamp CurrentTimestamp = Timestamps[i];
		FString WordContent = Contents[i];

		// Process each character in the content (old behavior)
		for (int32 CharIndex = 0; CharIndex < WordContent.Len(); CharIndex++)
		{
			FString Char = WordContent.Mid(CharIndex, 1);

			// Calculate end timestamp for this character
			FDreamMusicLyricTimestamp EndTimestamp = CalculateEndTimestamp(CurrentTimestamp, CharIndex, WordContent, i, Timestamps);

			// Create word entry for character
			FDreamMusicLyricWord Word(CurrentTimestamp, EndTimestamp, Char);
			OutWords.Add(Word);
			FullContent += Char;

			// Update timestamp for next character (if not last character in segment)
			if (CharIndex < WordContent.Len() - 1)
			{
				CurrentTimestamp = EndTimestamp;
			}
		}
	}

	return FullContent;
}

FDreamMusicLyricTimestamp FDreamLyricGroupProcessor::CalculateEndTimestamp(const FDreamMusicLyricTimestamp& StartTimestamp, int32 CharIndex, const FString& WordContent, int32 SegmentIndex, const TArray<FDreamMusicLyricTimestamp>& AllTimestamps)
{
	// If this is the last character in this content segment
	if (CharIndex == WordContent.Len() - 1)
	{
		// Use next timestamp if available
		if (SegmentIndex + 1 < AllTimestamps.Num())
		{
			return AllTimestamps[SegmentIndex + 1];
		}
		else
		{
			// Default duration for last character
			return CreateTimestampFromMs(StartTimestamp.ToMilliseconds() + 500);
		}
	}
	else
	{
		// For characters within the same segment, distribute time
		int32 CharDuration = 200; // Default 200ms per character
		if (SegmentIndex + 1 < AllTimestamps.Num())
		{
			int32 TotalDuration = AllTimestamps[SegmentIndex + 1].ToMilliseconds() - StartTimestamp.ToMilliseconds();
			int32 RemainingChars = WordContent.Len() - CharIndex;
			if (RemainingChars > 0)
			{
				CharDuration = FMath::Max(100, TotalDuration / RemainingChars);
			}
		}

		return CreateTimestampFromMs(StartTimestamp.ToMilliseconds() + CharDuration);
	}
}

FDreamMusicLyricTimestamp FDreamLyricGroupProcessor::CreateTimestampFromMs(int32 TotalMs)
{
	return FDreamMusicLyricTimestamp(
		TotalMs / 3600000,
		(TotalMs / 60000) % 60,
		(TotalMs / 1000) % 60,
		TotalMs % 1000
	);
}

void FDreamLyricGroupProcessor::AssignToField(FDreamMusicLyric& Lyric, const FString& Content, const TArray<FDreamMusicLyricWord>& Words, const FString& TargetField)
{
	if (TargetField == TEXT("Content"))
	{
		Lyric.Content = Content;
		Lyric.WordTimings = Words;
	}
	else if (TargetField == TEXT("Romanization"))
	{
		Lyric.Romanization = Content;
		Lyric.RomanizationWordTimings = Words;
	}
	else if (TargetField == TEXT("Translate"))
	{
		Lyric.Translate = Content;
		// Translation typically doesn't need word-by-word timing, so we don't assign Words
	}
}

FString FDreamLyricGroupProcessor::ExtractContentFromLine(const FString& Line)
{
	FRegexPattern Pattern(TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\](.*)"));
	FRegexMatcher Matcher(Pattern, Line);

	if (Matcher.FindNext())
	{
		return Matcher.GetCaptureGroup(4);
	}

	return FString();
}