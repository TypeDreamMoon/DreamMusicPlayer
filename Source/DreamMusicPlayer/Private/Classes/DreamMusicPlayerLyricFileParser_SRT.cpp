#include "Classes/DreamMusicPlayerLyricFileParser.h"
#include "DreamMusicPlayerCommon.h"
#include "DreamMusicPlayerLog.h"
#include "Engine/Engine.h"

void FDreamMusicPlayerLyricFileParser_SRT::Parse()
{
	if (Lines.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("SRT Parser: No lines to parse"));
		return;
	}

	FString Line;
	FDreamMusicLyric Lyric;
	int State = 0; // 0: number, 1: timestamp, 2: content

	for (const FString& SourceLine : Lines)
	{
		Line = SourceLine.TrimStartAndEnd();

		if (Line.IsEmpty())
		{
			if (Lyric.Timestamp.TotalMilliseconds() != 0 || Lyric.EndTimestamp.TotalMilliseconds() != 0)
			{
				ProcessText(Lyric);
				ParsedLyrics.Add(Lyric);
				Lyric = FDreamMusicLyric();
			}

			State = 0;
			continue;
		}

		if (State == 0)
		{
			// 跳过行号
			State = 1;
		}
		else if (State == 1)
		{
			FRegexPattern Pattern(TEXT("((\\d{2}:\\d{2}:\\d{2},\\d{3})\\s*-->\\s*(\\d{2}:\\d{2}:\\d{2},\\d{3}))"));
			FRegexMatcher Matcher(Pattern, Line);
			if (Matcher.FindNext())
			{
				ParseSRTTimestamp(Line, Lyric);
			}
			State = 2;
		}
		else if (State == 2)
		{
			if (!Lyric.Content.IsEmpty())
			{
				Lyric.Content += TEXT("\n"); // 保留多行内容，用换行符连接
			}
			Lyric.Content += Line;
		}
	}

	if (Lyric.Timestamp.TotalMilliseconds() != 0 || Lyric.EndTimestamp.TotalMilliseconds() != 0)
	{
		ProcessText(Lyric);
		ParsedLyrics.Add(Lyric);
	}
}

bool FDreamMusicPlayerLyricFileParser_SRT::ParseSRTTimestamp(const FString& TimestampLine, FDreamMusicLyric& OutLyric)
{
	// SRT format: "00:00:48,710 --> 00:00:58,770"
	FString StartTime, EndTime;

	if (!TimestampLine.Split(TEXT(" --> "), &StartTime, &EndTime))
	{
		return false;
	}

	StartTime = StartTime.TrimStartAndEnd();
	EndTime = EndTime.TrimStartAndEnd();

	// Parse start timestamp manually for SRT format
	OutLyric.Timestamp = ParseSRTTime(StartTime);

	// Parse end timestamp manually for SRT format  
	OutLyric.EndTimestamp = ParseSRTTime(EndTime);

	return true;
}

FDreamMusicLyricTimestamp FDreamMusicPlayerLyricFileParser_SRT::ParseSRTTime(const FString& TimeString)
{
	FDreamMusicLyricTimestamp Result;

	// SRT format: HH:MM:SS,mmm
	TArray<FString> TimeParts;
	TimeString.ParseIntoArray(TimeParts, TEXT(":"));

	if (TimeParts.Num() >= 3)
	{
		// Parse hours and minutes
		Result.Hours = FCString::Atoi(*TimeParts[0]);
		Result.Minute = FCString::Atoi(*TimeParts[1]);

		// Parse seconds and milliseconds (format: SS,mmm)
		FString SecondsAndMs = TimeParts[2];
		FString SecondsStr, MillisecondsStr;

		if (SecondsAndMs.Split(TEXT(","), &SecondsStr, &MillisecondsStr))
		{
			Result.Seconds = FCString::Atoi(*SecondsStr);
			Result.Millisecond = FCString::Atoi(*MillisecondsStr);
		}
		else if (SecondsAndMs.Split(TEXT("."), &SecondsStr, &MillisecondsStr))
		{
			// Handle alternate format with dot separator
			Result.Seconds = FCString::Atoi(*SecondsStr);
			Result.Millisecond = FCString::Atoi(*MillisecondsStr);
		}
		else
		{
			// No milliseconds, just seconds
			Result.Seconds = FCString::Atoi(*SecondsAndMs);
			Result.Millisecond = 0;
		}
	}

	return Result;
}

// FString FDreamMusicPlayerLyricFileParser_SRT::CleanSRTContent(const FString& RawContent)
// {
// 	FString CleanedContent = RawContent;
//
// 	// Remove common SRT formatting tags
// 	CleanedContent = CleanedContent.Replace(TEXT("<i>"), TEXT(""));
// 	CleanedContent = CleanedContent.Replace(TEXT("</i>"), TEXT(""));
// 	CleanedContent = CleanedContent.Replace(TEXT("<b>"), TEXT(""));
// 	CleanedContent = CleanedContent.Replace(TEXT("</b>"), TEXT(""));
// 	CleanedContent = CleanedContent.Replace(TEXT("<u>"), TEXT(""));
// 	CleanedContent = CleanedContent.Replace(TEXT("</u>"), TEXT(""));
// 	CleanedContent = CleanedContent.Replace(TEXT("</font>"), TEXT(""));
//
// 	// Remove font color tags using simple string operations
// 	// Pattern: <font color="...">
// 	int32 StartPos = 0;
// 	while ((StartPos = CleanedContent.Find(TEXT("<font color=\""), StartPos)) != INDEX_NONE)
// 	{
// 		int32 EndPos = CleanedContent.Find(TEXT("\">"), StartPos);
// 		if (EndPos != INDEX_NONE)
// 		{
// 			CleanedContent.RemoveAt(StartPos, EndPos - StartPos + 2);
// 		}
// 		else
// 		{
// 			break;
// 		}
// 	}
// 	
// 	StartPos = 0;
// 	while ((StartPos = CleanedContent.Find(TEXT("{"), StartPos)) != INDEX_NONE)
// 	{
// 		int32 EndPos = CleanedContent.Find(TEXT("}"), StartPos);
// 		if (EndPos != INDEX_NONE)
// 		{
// 			CleanedContent.RemoveAt(StartPos, EndPos - StartPos + 1);
// 		}
// 		else
// 		{
// 			StartPos++;
// 		}
// 	}
//
// 	return CleanedContent.TrimStartAndEnd();
// }

void FDreamMusicPlayerLyricFileParser_SRT::ProcessText(FDreamMusicLyric& Lyric)
{
	TArray<FString> ProcessLines;
	Lyric.Content.ParseIntoArrayLines(ProcessLines, false); // false 表示保留空行

	// 清空原有内容，准备重新填充
	Lyric.Content.Empty();
	Lyric.Romanization.Empty();
	Lyric.Translate.Empty();

	// 如果只有一行，则直接视为歌词内容（原文）
	if (ProcessLines.Num() == 1)
	{
		Lyric.Content = ProcessLines[0];
	}
	else
	{
		// 根据歌词行类型处理内容
		switch (LineType)
		{
		case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
			if (ProcessLines.IsValidIndex(0)) Lyric.Romanization = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Content = ProcessLines[1];
			if (ProcessLines.IsValidIndex(2)) Lyric.Translate = ProcessLines[2];
			break;

		case EDreamMusicPlayerLyricParseLineType::Romanization_Translation_Lyric:
			if (ProcessLines.IsValidIndex(0)) Lyric.Romanization = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Translate = ProcessLines[1];
			if (ProcessLines.IsValidIndex(2)) Lyric.Content = ProcessLines[2];
			break;

		case EDreamMusicPlayerLyricParseLineType::Translation_Romanization_Lyric:
			if (ProcessLines.IsValidIndex(0)) Lyric.Translate = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Romanization = ProcessLines[1];
			if (ProcessLines.IsValidIndex(2)) Lyric.Content = ProcessLines[2];
			break;

		case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
			if (ProcessLines.IsValidIndex(0)) Lyric.Translate = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Content = ProcessLines[1];
			if (ProcessLines.IsValidIndex(2)) Lyric.Romanization = ProcessLines[2];
			break;

		case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
			if (ProcessLines.IsValidIndex(0)) Lyric.Content = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Romanization = ProcessLines[1];
			if (ProcessLines.IsValidIndex(2)) Lyric.Translate = ProcessLines[2];
			break;

		case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
			if (ProcessLines.IsValidIndex(0)) Lyric.Romanization = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Content = ProcessLines[1];
			break;

		case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
			if (ProcessLines.IsValidIndex(0)) Lyric.Content = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Romanization = ProcessLines[1];
			break;

		case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
			if (ProcessLines.IsValidIndex(0)) Lyric.Translate = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Content = ProcessLines[1];
			break;

		case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
			if (ProcessLines.IsValidIndex(0)) Lyric.Content = ProcessLines[0];
			if (ProcessLines.IsValidIndex(1)) Lyric.Translate = ProcessLines[1];
			break;

		case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
		default:
			// 默认情况下，将所有行合并为 Content（用换行符连接）
			Lyric.Content = FString::Join(ProcessLines, TEXT("\n"));
			break;
		}
	}

	DMP_LOG(Log, TEXT("Lyric processed - Content: %s, Romanization: %s, Translate: %s"),
	        *Lyric.Content, *Lyric.Romanization, *Lyric.Translate);
}
