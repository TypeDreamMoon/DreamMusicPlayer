#include "LyricParser/DreamMusicPlayerLyricFileParser.h"
#include "DreamMusicPlayerLog.h"

// Enhanced FDreamMusicPlayerLyricFileParser_LRC::Parse() method
void FDreamMusicPlayerLyricFileParser_LRC::Parse()
{
	ParsedLyrics.Empty();

	// Collect valid lines
	TArray<FString> ValidLines;
	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();
		if (TrimmedLine.IsEmpty() || IsMetadataLine(TrimmedLine))
		{
			continue;
		}

		if (TrimmedLine.StartsWith(TEXT("[")) && TrimmedLine.Contains(TEXT("]")))
		{
			ValidLines.Add(TrimmedLine);
		}
	}

	// Group lines by timestamp
	TMap<FString, TArray<FString>> LinesByTimestamp;
	for (const FString& Line : ValidLines)
	{
		FString Timestamp = ExtractTimestampFromLine(Line);
		if (!Timestamp.IsEmpty())
		{
			LinesByTimestamp.FindOrAdd(Timestamp).Add(Line);
		}
	}

	// Process each group using the GroupProcessor
	for (auto& Pair : LinesByTimestamp)
	{
		FDreamMusicLyric Lyric = CreateLyricFromLine(Pair.Value[0]);
		GroupProcessor.ProcessGroup(Pair.Value, Lyric);
		ParsedLyrics.Add(Lyric);
	}

	// Sort by timestamp
	ParsedLyrics.Sort([](const FDreamMusicLyric& A, const FDreamMusicLyric& B)
	{
		return A.StartTimestamp.ToMilliseconds() < B.StartTimestamp.ToMilliseconds();
	});

	// **FIX: Calculate proper end timestamps for LRC lyrics**
	UpdateEndTimestampsBasedOnWordTimings();
}

// Add this new method to FDreamMusicPlayerLyricFileParser_LRC class
void FDreamMusicPlayerLyricFileParser_LRC::UpdateEndTimestampsBasedOnWordTimings()
{
	for (int32 i = 0; i < ParsedLyrics.Num(); i++)
	{
		FDreamMusicLyric& CurrentLyric = ParsedLyrics[i];
		
		// Calculate end timestamp based on word timings if available
		FDreamMusicLyricTimestamp CalculatedEndTime;
		bool bHasWordTimings = false;

		// Check both regular and romanization word timings
		if (!CurrentLyric.IsWordsEmpty())
		{
			CalculatedEndTime = CurrentLyric.WordTimings.Last().EndTimestamp;
			bHasWordTimings = true;
		}
		else if (!CurrentLyric.IsRomanizationWordsEmpty())
		{
			CalculatedEndTime = CurrentLyric.RomanizationWordTimings.Last().EndTimestamp;
			bHasWordTimings = true;
		}

		if (bHasWordTimings)
		{
			// Use calculated end time from word timings
			CurrentLyric.EndTimestamp = CalculatedEndTime;
		}
		else
		{
			// Fallback: Use next lyric's start time or default duration
			if (i + 1 < ParsedLyrics.Num())
			{
				// Use next lyric's start time as this lyric's end time
				CurrentLyric.EndTimestamp = ParsedLyrics[i + 1].StartTimestamp;
			}
			else
			{
				// For last lyric, add default duration
				int32 DefaultDurationMs = 3000;
				int32 EndMs = CurrentLyric.StartTimestamp.ToMilliseconds() + DefaultDurationMs;
				CurrentLyric.EndTimestamp = FDreamMusicLyricTimestamp(
					EndMs / 3600000,
					(EndMs / 60000) % 60,
					(EndMs / 1000) % 60,
					EndMs % 1000);
			}
		}
	}
}

FDreamMusicLyric FDreamMusicPlayerLyricFileParser_LRC::CreateLyricFromLine(const FString& Line)
{
	FDreamMusicLyric Lyric;

	FRegexPattern Pattern(TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\](.*)"));
	FRegexMatcher Matcher(Pattern, Line);

	if (Matcher.FindNext())
	{
		int32 Minutes = FCString::Atoi(*Matcher.GetCaptureGroup(1));
		int32 Seconds = FCString::Atoi(*Matcher.GetCaptureGroup(2));
		FString MillisecondsStr = Matcher.GetCaptureGroup(3);

		int32 Milliseconds = (MillisecondsStr.Len() == 2) ? FCString::Atoi(*MillisecondsStr) * 10 : FCString::Atoi(*MillisecondsStr);
		Lyric.StartTimestamp = FDreamMusicLyricTimestamp(Minutes, Seconds, Milliseconds);
		
		// Set default end time
		int32 EndMs = Lyric.StartTimestamp.ToMilliseconds() + 3000;
		Lyric.EndTimestamp = FDreamMusicLyricTimestamp(
			EndMs / 3600000,
			(EndMs / 60000) % 60,
			(EndMs / 1000) % 60,
			EndMs % 1000);
	}

	return Lyric;
}

FString FDreamMusicPlayerLyricFileParser_LRC::ExtractTimestampFromLine(const FString& Line)
{
	FRegexPattern Pattern(TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\]"));
	FRegexMatcher Matcher(Pattern, Line);

	if (Matcher.FindNext())
	{
		return FString::Printf(TEXT("%s:%s.%s"),
							   *Matcher.GetCaptureGroup(1),
							   *Matcher.GetCaptureGroup(2),
							   *Matcher.GetCaptureGroup(3));
	}

	return FString();
}

bool FDreamMusicPlayerLyricFileParser_LRC::IsMetadataLine(const FString& Line) const
{
	if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")) && Line.Contains(TEXT(":")))
	{
		FString BeforeColon;
		FString AfterColon;
		if (Line.Split(TEXT(":"), &BeforeColon, &AfterColon))
		{
			if (BeforeColon.Len() == 3 && BeforeColon.StartsWith(TEXT("[")))
			{
				FString Identifier = BeforeColon.Mid(1);
				if (Identifier == TEXT("ar") || Identifier == TEXT("ti") ||
					Identifier == TEXT("al") || Identifier == TEXT("by") ||
					Identifier == TEXT("offset") || Identifier == TEXT("tool") ||
					Identifier == TEXT("re") || Identifier == TEXT("ve"))
				{
					return true;
				}
			}
		}
	}
	return false;
}