#include "Classes/DreamMusicPlayerLyricFileParser.h"
#include "DreamMusicPlayerLog.h"

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
		return A.Timestamp.ToMilliseconds() < B.Timestamp.ToMilliseconds();
	});
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
		Lyric.Timestamp = FDreamMusicLyricTimestamp(Minutes, Seconds, Milliseconds);
		
		// Set default end time
		int32 EndMs = Lyric.Timestamp.ToMilliseconds() + 3000;
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