#include "Classes/DreamMusicPlayerLyricFileParser.h"
#include "DreamMusicPlayerLog.h"

void FDreamMusicPlayerLyricFileParser_LRC::Parse()
{
	// 先收集所有有效行（包含时间戳的行）
	TArray<FString> ValidLines;
	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();

		// 跳过空行
		if (TrimmedLine.IsEmpty())
		{
			continue;
		}

		// 跳过标准的元数据行（如 [ar:Artist]）但保留可能包含歌词的行
		if (IsMetadataLine(TrimmedLine))
		{
			continue;
		}

		// 只保留包含时间戳的行
		if (TrimmedLine.StartsWith(TEXT("[")) && TrimmedLine.Contains(TEXT("]")))
		{
			ValidLines.Add(TrimmedLine);
		}
	}

	// 按时间戳分组处理
	TMap<FString, TArray<FString>> LinesByTimestamp;

	// 将具有相同时间戳的行分组
	for (const FString& Line : ValidLines)
	{
		FString Timestamp = ExtractTimestampFromLine(Line);
		if (!Timestamp.IsEmpty())
		{
			LinesByTimestamp.FindOrAdd(Timestamp).Add(Line);
		}
	}

	// 处理每个时间戳组
	for (auto& Pair : LinesByTimestamp)
	{
		ProcessTimestampGroup(Pair.Value);
	}
}

void FDreamMusicPlayerLyricFileParser_LRC::ProcessText(FDreamMusicLyric& Lyric)
{
	// LRC格式通常不需要额外的文本处理
}

void FDreamMusicPlayerLyricFileParser_LRC::ParseLinesAsTriple(const TArray<FString>& InLines, int32 LyricIndex)
{
	// 每3行组成一组
	for (int32 i = 0; i <= InLines.Num() - 3; i += 3)
	{
		// 所有行应该有相同的时间戳，使用指定索引行的时间戳
		FDreamMusicLyric Lyric = CreateLyricFromLine(InLines[i + LyricIndex]);

		// 根据行类型分配内容
		switch (LineType)
		{
		case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
			Lyric.Romanization = ExtractContentFromLine(InLines[i]); // 第1行：罗马音
			Lyric.Content = ExtractContentFromLine(InLines[i + 1]); // 第2行：歌词
			Lyric.Translate = ExtractContentFromLine(InLines[i + 2]); // 第3行：翻译
			break;
		case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
			Lyric.Content = ExtractContentFromLine(InLines[i]); // 第1行：歌词
			Lyric.Romanization = ExtractContentFromLine(InLines[i + 1]); // 第2行：罗马音
			Lyric.Translate = ExtractContentFromLine(InLines[i + 2]); // 第3行：翻译
			break;
		case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
			Lyric.Translate = ExtractContentFromLine(InLines[i]); // 第1行：翻译
			Lyric.Content = ExtractContentFromLine(InLines[i + 1]); // 第2行：歌词
			Lyric.Romanization = ExtractContentFromLine(InLines[i + 2]); // 第3行：罗马音
			break;
		}

		// 根据解析模式处理歌词内容
		ProcessLyricContent(Lyric, InLines[i + LyricIndex]);
		ParsedLyrics.Add(Lyric);
	}
}


void FDreamMusicPlayerLyricFileParser_LRC::ParseLinesAsPair(const TArray<FString>& InLines, int32 LyricIndex)
{
	// 每2行组成一组
	for (int32 i = 0; i <= InLines.Num() - 2; i += 2)
	{
		// 所有行应该有相同的时间戳，使用指定索引行的时间戳
		FDreamMusicLyric Lyric = CreateLyricFromLine(InLines[i + LyricIndex]);

		// 根据行类型分配内容
		switch (LineType)
		{
		case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
			Lyric.Romanization = ExtractContentFromLine(InLines[i]); // 第1行：罗马音
			Lyric.Content = ExtractContentFromLine(InLines[i + 1]); // 第2行：歌词
			break;
		case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
			Lyric.Content = ExtractContentFromLine(InLines[i]); // 第1行：歌词
			Lyric.Romanization = ExtractContentFromLine(InLines[i + 1]); // 第2行：罗马音
			break;
		case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
			Lyric.Translate = ExtractContentFromLine(InLines[i]); // 第1行：翻译
			Lyric.Content = ExtractContentFromLine(InLines[i + 1]); // 第2行：歌词
			break;
		case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
			Lyric.Content = ExtractContentFromLine(InLines[i]); // 第1行：歌词
			Lyric.Translate = ExtractContentFromLine(InLines[i + 1]); // 第2行：翻译
			break;
		}

		// 根据解析模式处理歌词内容
		ProcessLyricContent(Lyric, InLines[i + LyricIndex]);
		ParsedLyrics.Add(Lyric);
	}
}

void FDreamMusicPlayerLyricFileParser_LRC::ParseLinesAsSingle(const TArray<FString>& InLines)
{
	for (const FString& Line : InLines)
	{
		FDreamMusicLyric Lyric = CreateLyricFromLine(Line);
		ProcessLyricContent(Lyric, Line);
		ParsedLyrics.Add(Lyric);
	}
}

FDreamMusicLyric FDreamMusicPlayerLyricFileParser_LRC::CreateLyricFromLine(const FString& Line)
{
	FDreamMusicLyric Lyric;

	// 解析时间戳
	FRegexPattern Pattern(TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\](.*)"));
	FRegexMatcher Matcher(Pattern, Line);

	if (Matcher.FindNext())
	{
		int32 Minutes = FCString::Atoi(*Matcher.GetCaptureGroup(1));
		int32 Seconds = FCString::Atoi(*Matcher.GetCaptureGroup(2));
		FString MillisecondsStr = Matcher.GetCaptureGroup(3);
		FString Content = Matcher.GetCaptureGroup(4);

		int32 Milliseconds = 0;
		if (MillisecondsStr.Len() == 2)
		{
			Milliseconds = FCString::Atoi(*MillisecondsStr) * 10; // 转换为3位毫秒
		}
		else
		{
			Milliseconds = FCString::Atoi(*MillisecondsStr);
		}

		Lyric.Timestamp = FDreamMusicLyricTimestamp(Minutes, Seconds, Milliseconds);

		// 设置一个默认的结束时间
		int32 EndMs = Lyric.Timestamp.TotalMilliseconds() + 3000;
		Lyric.EndTimestamp = FDreamMusicLyricTimestamp(
			EndMs / 3600000,
			(EndMs / 60000) % 60,
			(EndMs / 1000) % 60,
			EndMs % 1000);
	}

	return Lyric;
}

FString FDreamMusicPlayerLyricFileParser_LRC::ExtractContentFromLine(const FString& Line)
{
	FRegexPattern Pattern(TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\](.*)"));
	FRegexMatcher Matcher(Pattern, Line);

	if (Matcher.FindNext())
	{
		return Matcher.GetCaptureGroup(4).TrimStartAndEnd();
	}

	return FString();
}


void FDreamMusicPlayerLyricFileParser_LRC::ProcessLyricContent(FDreamMusicLyric& Lyric, const FString& Line)
{
	// 根据解析模式处理歌词内容
	switch (ParseMethod)
	{
	case EDreamMusicPlayerLrcLyricType::WordByWord:
		ProcessWordByWordContent(Lyric, Line);
		break;
	case EDreamMusicPlayerLrcLyricType::ESLyric:
		ProcessESLyricContent(Lyric, Line);
		break;
	case EDreamMusicPlayerLrcLyricType::LineByLine:
	default:
		// LineByLine模式下，如果Content还没有设置，则从Line中提取
		if (Lyric.Content.IsEmpty())
		{
			Lyric.Content = ExtractContentFromLine(Line);
		}
		break;
	}
}

void FDreamMusicPlayerLyricFileParser_LRC::ProcessWordByWordContent(FDreamMusicLyric& Lyric, const FString& Line)
{
	// Parse WordByWord format: [00:48.710]风 [00:50.560]触[00:51.060]れ[00:51.270]る [00:53.570]霍[00:53.920]希[00:54.800]の [00:56.620]愿[00:57.270]い[00:58.770]

	// Extract all timestamp-word pairs
	FRegexPattern WordPattern(TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\]([^\\[]*)"));
	FRegexMatcher WordMatcher(WordPattern, Line);

	TArray<FDreamMusicLyricTimestamp> Timestamps;
	TArray<FString> Contents;

	// Collect all timestamps and their corresponding content
	while (WordMatcher.FindNext())
	{
		int32 Minutes = FCString::Atoi(*WordMatcher.GetCaptureGroup(1));
		int32 Seconds = FCString::Atoi(*WordMatcher.GetCaptureGroup(2));
		FString MillisecondsStr = WordMatcher.GetCaptureGroup(3);
		FString WordContent = WordMatcher.GetCaptureGroup(4);

		int32 Milliseconds = (MillisecondsStr.Len() == 2) ? FCString::Atoi(*MillisecondsStr) * 10 : FCString::Atoi(*MillisecondsStr);
		FDreamMusicLyricTimestamp Timestamp(Minutes, Seconds, Milliseconds);

		Timestamps.Add(Timestamp);
		Contents.Add(WordContent);
	}

	if (Timestamps.Num() == 0)
	{
		return; // No valid WordByWord content found
	}

	// Build words with proper timing
	TArray<FDreamMusicLyricWord> Words;
	FString FullContent;

	for (int32 i = 0; i < Timestamps.Num(); i++)
	{
		FDreamMusicLyricTimestamp StartTimestamp = Timestamps[i];
		FString WordContent = Contents[i].TrimStartAndEnd();

		// Skip empty content
		if (WordContent.IsEmpty())
		{
			continue;
		}

		// Process each character in the content
		for (int32 CharIndex = 0; CharIndex < WordContent.Len(); CharIndex++)
		{
			FString Char = WordContent.Mid(CharIndex, 1);

			// Skip processing spaces but add them to full content
			if (Char == TEXT(" "))
			{
				FullContent += Char;
				continue;
			}

			// Calculate end timestamp for this character
			FDreamMusicLyricTimestamp EndTimestamp;

			// If this is the last character in this content segment
			if (CharIndex == WordContent.Len() - 1)
			{
				// Use next timestamp if available, otherwise add default duration
				if (i + 1 < Timestamps.Num())
				{
					EndTimestamp = Timestamps[i + 1];
				}
				else
				{
					// Default duration for last character
					int32 EndMs = StartTimestamp.TotalMilliseconds() + 500;
					EndTimestamp = FDreamMusicLyricTimestamp(
						EndMs / 3600000,
						(EndMs / 60000) % 60,
						(EndMs / 1000) % 60,
						EndMs % 1000
					);
				}
			}
			else
			{
				// For characters within the same content segment, 
				// distribute time evenly
				int32 CharDuration = 200; // 200ms per character as default
				if (i + 1 < Timestamps.Num())
				{
					int32 TotalDuration = Timestamps[i + 1].TotalMilliseconds() - StartTimestamp.TotalMilliseconds();
					int32 RemainingChars = WordContent.Len() - CharIndex;
					if (RemainingChars > 0)
					{
						CharDuration = FMath::Max(100, TotalDuration / RemainingChars); // Minimum 100ms per char
					}
				}

				int32 EndMs = StartTimestamp.TotalMilliseconds() + CharDuration;
				EndTimestamp = FDreamMusicLyricTimestamp(
					EndMs / 3600000,
					(EndMs / 60000) % 60,
					(EndMs / 1000) % 60,
					EndMs % 1000
				);

				// Update start timestamp for next character
				if (CharIndex < WordContent.Len() - 1)
				{
					StartTimestamp = EndTimestamp;
				}
			}

			FDreamMusicLyricWord Word(StartTimestamp, EndTimestamp, Char);
			Words.Add(Word);
			FullContent += Char;
		}
	}

	// Store the content and word timings - assignment will be handled by ProcessTimestampGroup
	Lyric.Content = FullContent;
	Lyric.WordTimings = Words;
}

void FDreamMusicPlayerLyricFileParser_LRC::ProcessESLyricContent(FDreamMusicLyric& Lyric, const FString& Line)
{
	// Parse ESLyric format: [00:48.710]<00:48.710>风 <00:50.560>触<00:51.060>れ<00:51.270>る <00:53.570>霍<00:53.920>希<00:54.800>の <00:56.620>愿<00:57.270>い<00:58.770>

	// First extract all timestamp-content pairs
	FRegexPattern ESLyricPattern(TEXT("<(\\d{2}):(\\d{2})[.:](\\d{2,3})>([^<]*)"));
	FRegexMatcher ESLyricMatcher(ESLyricPattern, Line);

	TArray<FDreamMusicLyricTimestamp> Timestamps;
	TArray<FString> Contents;

	// Collect all timestamps and their corresponding content
	while (ESLyricMatcher.FindNext())
	{
		int32 Minutes = FCString::Atoi(*ESLyricMatcher.GetCaptureGroup(1));
		int32 Seconds = FCString::Atoi(*ESLyricMatcher.GetCaptureGroup(2));
		FString MillisecondsStr = ESLyricMatcher.GetCaptureGroup(3);
		FString WordContent = ESLyricMatcher.GetCaptureGroup(4);

		int32 Milliseconds = (MillisecondsStr.Len() == 2) ? FCString::Atoi(*MillisecondsStr) * 10 : FCString::Atoi(*MillisecondsStr);
		FDreamMusicLyricTimestamp Timestamp(Minutes, Seconds, Milliseconds);

		Timestamps.Add(Timestamp);
		Contents.Add(WordContent);
	}

	if (Timestamps.Num() == 0)
	{
		return; // No valid ESLyric content found
	}

	// Build words with proper timing
	TArray<FDreamMusicLyricWord> Words;
	FString FullContent;

	for (int32 i = 0; i < Timestamps.Num(); i++)
	{
		FDreamMusicLyricTimestamp StartTimestamp = Timestamps[i];
		FString WordContent = Contents[i];

		// Process each character in the content
		for (int32 CharIndex = 0; CharIndex < WordContent.Len(); CharIndex++)
		{
			FString Char = WordContent.Mid(CharIndex, 1);

			// Skip processing spaces but add them to full content
			if (Char == TEXT(" "))
			{
				FullContent += Char;
				continue;
			}

			// Calculate end timestamp for this character
			FDreamMusicLyricTimestamp EndTimestamp;

			// If this is the last character in this content segment
			if (CharIndex == WordContent.Len() - 1)
			{
				// Use next timestamp if available, otherwise add default duration
				if (i + 1 < Timestamps.Num())
				{
					EndTimestamp = Timestamps[i + 1];
				}
				else
				{
					// Default duration for last character
					int32 EndMs = StartTimestamp.TotalMilliseconds() + 500;
					EndTimestamp = FDreamMusicLyricTimestamp(
						EndMs / 3600000,
						(EndMs / 60000) % 60,
						(EndMs / 1000) % 60,
						EndMs % 1000
					);
				}
			}
			else
			{
				// For characters within the same content segment, 
				// distribute time evenly or use a small default duration
				int32 CharDuration = 200; // 200ms per character as default
				if (i + 1 < Timestamps.Num())
				{
					int32 TotalDuration = Timestamps[i + 1].TotalMilliseconds() - StartTimestamp.TotalMilliseconds();
					int32 RemainingChars = WordContent.Len() - CharIndex;
					CharDuration = FMath::Max(100, TotalDuration / RemainingChars); // Minimum 100ms per char
				}

				int32 EndMs = StartTimestamp.TotalMilliseconds() + CharDuration;
				EndTimestamp = FDreamMusicLyricTimestamp(
					EndMs / 3600000,
					(EndMs / 60000) % 60,
					(EndMs / 1000) % 60,
					EndMs % 1000
				);

				// Update start timestamp for next character
				if (CharIndex < WordContent.Len() - 1)
				{
					StartTimestamp = EndTimestamp;
				}
			}

			FDreamMusicLyricWord Word(StartTimestamp, EndTimestamp, Char);
			Words.Add(Word);
			FullContent += Char;
		}
	}

	// Store the parsed content and word timings in temporary variables
	// The actual assignment will be handled by ProcessTimestampGroup based on line order
	if (!FullContent.IsEmpty())
	{
		// For ESLyric mode, we need to store this content temporarily
		// and let the group processing logic assign it to the correct field
		// based on the LineType and line index within the group

		// Since we don't know which line this is in the group yet,
		// we'll store it as Content temporarily and let ProcessTimestampGroup
		// handle the proper assignment based on line order
		Lyric.Content = FullContent;
		Lyric.WordTimings = Words;
	}
}


void FDreamMusicPlayerLyricFileParser_LRC::ProcessTimestampGroup(const TArray<FString>& LinesInGroup)
{
	if (LinesInGroup.Num() == 0)
		return;

	// Create lyric object using the first line's timestamp
	FDreamMusicLyric Lyric = CreateLyricFromLine(LinesInGroup[0]);

	// Determine expected number of lines based on LineType
	int32 ExpectedLines = 1;
	switch (LineType)
	{
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
		ExpectedLines = 3;
		break;
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
		ExpectedLines = 2;
		break;
	case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
	default:
		ExpectedLines = 1;
		break;
	}

	// Handle different cases based on actual vs expected lines
	int32 ActualLines = LinesInGroup.Num();

	if (ActualLines >= ExpectedLines)
	{
		// We have enough lines, assign according to LineType
		if (ParseMethod == EDreamMusicPlayerLrcLyricType::ESLyric)
		{
			// For ESLyric, process each line individually and assign to correct fields
			switch (LineType)
			{
			case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Romanization"));
				if (LinesInGroup.Num() >= 2) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				if (LinesInGroup.Num() >= 3) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[2], TEXT("Translate"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				if (LinesInGroup.Num() >= 2) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Romanization"));
				if (LinesInGroup.Num() >= 3) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[2], TEXT("Translate"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Translate"));
				if (LinesInGroup.Num() >= 2) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				if (LinesInGroup.Num() >= 3) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[2], TEXT("Romanization"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Romanization"));
				if (LinesInGroup.Num() >= 2) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				if (LinesInGroup.Num() >= 2) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Romanization"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Translate"));
				if (LinesInGroup.Num() >= 2) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				if (LinesInGroup.Num() >= 2) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Translate"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
			default:
				if (LinesInGroup.Num() >= 1) ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				break;
			}
		}
		else if (ParseMethod == EDreamMusicPlayerLrcLyricType::WordByWord)
		{
			// For WordByWord, process each line individually and assign to correct fields
			switch (LineType)
			{
			case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Romanization"));
				if (LinesInGroup.Num() >= 2) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				if (LinesInGroup.Num() >= 3) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[2], TEXT("Translate"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				if (LinesInGroup.Num() >= 2) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Romanization"));
				if (LinesInGroup.Num() >= 3) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[2], TEXT("Translate"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Translate"));
				if (LinesInGroup.Num() >= 2) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				if (LinesInGroup.Num() >= 3) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[2], TEXT("Romanization"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Romanization"));
				if (LinesInGroup.Num() >= 2) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				if (LinesInGroup.Num() >= 2) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Romanization"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Translate"));
				if (LinesInGroup.Num() >= 2) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Content"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				if (LinesInGroup.Num() >= 2) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Translate"));
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
			default:
				if (LinesInGroup.Num() >= 1) ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
				break;
			}
		}
		else
		{
			// For non-ESLyric modes, use existing assignment functions
			switch (LineType)
			{
			case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
				AssignContentByType_RLT(Lyric, LinesInGroup);
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
				AssignContentByType_LRT(Lyric, LinesInGroup);
				break;
			case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
				AssignContentByType_TLR(Lyric, LinesInGroup);
				break;
			case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
				AssignContentByType_RL(Lyric, LinesInGroup);
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
				AssignContentByType_LR(Lyric, LinesInGroup);
				break;
			case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
				AssignContentByType_TL(Lyric, LinesInGroup);
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
				AssignContentByType_LT(Lyric, LinesInGroup);
				break;
			case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
			default:
				AssignContentByType_LO(Lyric, LinesInGroup);
				break;
			}
		}
	}
	else if (ActualLines == 2 && ExpectedLines == 3)
	{
		// Fallback to 2-line mode: first line = lyrics, second line = translation
		if (ParseMethod == EDreamMusicPlayerLrcLyricType::ESLyric)
		{
			ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
			ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Translate"));
		}
		else if (ParseMethod == EDreamMusicPlayerLrcLyricType::WordByWord)
		{
			ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
			ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[1], TEXT("Translate"));
		}
		else
		{
			Lyric.Content = ExtractContentFromLine(LinesInGroup[0]);
			Lyric.Translate = ExtractContentFromLine(LinesInGroup[1]);
		}
	}
	else if (ActualLines == 1)
	{
		// Fallback to 1-line mode: only lyrics, no translation
		if (ParseMethod == EDreamMusicPlayerLrcLyricType::ESLyric)
		{
			ProcessESLyricContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
		}
		else if (ParseMethod == EDreamMusicPlayerLrcLyricType::WordByWord)
		{
			ProcessWordByWordContentForSpecificField(Lyric, LinesInGroup[0], TEXT("Content"));
		}
		else
		{
			Lyric.Content = ExtractContentFromLine(LinesInGroup[0]);
		}
	}

	ParsedLyrics.Add(Lyric);
}

int32 FDreamMusicPlayerLyricFileParser_LRC::GetMainLyricIndex() const
{
	// 根据LineType确定哪一行是主歌词行（原文）
	switch (LineType)
	{
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric_Translation:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization_Translation:
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric_Romanization:
		return 1; // 第二行是歌词
	case EDreamMusicPlayerLyricParseLineType::Romanization_Lyric:
	case EDreamMusicPlayerLyricParseLineType::Translation_Lyric:
		return 1; // 第二行是歌词
	case EDreamMusicPlayerLyricParseLineType::Lyric_Romanization:
	case EDreamMusicPlayerLyricParseLineType::Lyric_Translation:
		return 0; // 第一行是歌词
	case EDreamMusicPlayerLyricParseLineType::Lyric_Only:
	default:
		return 0; // 第一行是歌词
	}
}

bool FDreamMusicPlayerLyricFileParser_LRC::IsMetadataLine(const FString& Line) const
{
	// 检查是否为标准元数据行，如 [ar:Artist], [ti:Title] 等
	// 这些行具有格式 [xx:yyy]，其中xx是2个字母的标识符
	if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")) && Line.Contains(TEXT(":")))
	{
		// 提取第一个:之前的部分
		FString BeforeColon;
		FString AfterColon;
		if (Line.Split(TEXT(":"), &BeforeColon, &AfterColon))
		{
			// 如果冒号前的部分（包括开头的[）长度为3，则为元数据行
			// 例如 [ar:YuNi] -> [ar 长度为3
			if (BeforeColon.Len() == 3 && BeforeColon.StartsWith(TEXT("[")))
			{
				// 额外检查标识符是否为常见的元数据标识符
				FString Identifier = BeforeColon.Mid(1); // 去掉开头的[
				if (Identifier == TEXT("ar") || Identifier == TEXT("ti") ||
					Identifier == TEXT("al") || Identifier == TEXT("by") ||
					Identifier == TEXT("offset") || Identifier == TEXT("tool") ||
					Identifier == TEXT("re"))
				{
					return true;
				}
			}
		}
	}
	return false;
}


void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_RLT(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Romanization_Lyric_Translation: 罗马音-歌词-翻译
	if (GroupLines.Num() >= 1) Lyric.Romanization = ExtractContentFromLine(GroupLines[0]);
	if (GroupLines.Num() >= 2) Lyric.Content = ExtractContentFromLine(GroupLines[1]);
	if (GroupLines.Num() >= 3) Lyric.Translate = ExtractContentFromLine(GroupLines[2]);
}

void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_LRT(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Lyric_Romanization_Translation: 歌词-罗马音-翻译
	if (GroupLines.Num() >= 1) Lyric.Content = ExtractContentFromLine(GroupLines[0]);
	if (GroupLines.Num() >= 2) Lyric.Romanization = ExtractContentFromLine(GroupLines[1]);
	if (GroupLines.Num() >= 3) Lyric.Translate = ExtractContentFromLine(GroupLines[2]);
}

void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_TLR(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Translation_Lyric_Romanization: 翻译-歌词-罗马音
	if (GroupLines.Num() >= 1) Lyric.Translate = ExtractContentFromLine(GroupLines[0]);
	if (GroupLines.Num() >= 2) Lyric.Content = ExtractContentFromLine(GroupLines[1]);
	if (GroupLines.Num() >= 3) Lyric.Romanization = ExtractContentFromLine(GroupLines[2]);
}

void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_RL(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Romanization_Lyric: 罗马音-歌词
	if (GroupLines.Num() >= 1) Lyric.Romanization = ExtractContentFromLine(GroupLines[0]);
	if (GroupLines.Num() >= 2) Lyric.Content = ExtractContentFromLine(GroupLines[1]);
}

void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_LR(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Lyric_Romanization: 歌词-罗马音
	if (GroupLines.Num() >= 1) Lyric.Content = ExtractContentFromLine(GroupLines[0]);
	if (GroupLines.Num() >= 2) Lyric.Romanization = ExtractContentFromLine(GroupLines[1]);
}

void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_TL(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Translation_Lyric: 翻译-歌词
	if (GroupLines.Num() >= 1) Lyric.Translate = ExtractContentFromLine(GroupLines[0]);
	if (GroupLines.Num() >= 2) Lyric.Content = ExtractContentFromLine(GroupLines[1]);
}

void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_LT(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Lyric_Translation: 歌词-翻译
	if (GroupLines.Num() >= 1) Lyric.Content = ExtractContentFromLine(GroupLines[0]);
	if (GroupLines.Num() >= 2) Lyric.Translate = ExtractContentFromLine(GroupLines[1]);
}

void FDreamMusicPlayerLyricFileParser_LRC::AssignContentByType_LO(FDreamMusicLyric& Lyric, const TArray<FString>& GroupLines)
{
	// Lyric_Only: 只有歌词
	if (GroupLines.Num() >= 1) Lyric.Content = ExtractContentFromLine(GroupLines[0]);
}

void FDreamMusicPlayerLyricFileParser_LRC::ProcessESLyricContentForSpecificField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField)
{
	// Extract word timings using the same logic as before
	FRegexPattern ESLyricPattern(TEXT("<(\\d{2}):(\\d{2})[.:](\\d{2,3})>([^<]*)"));
	FRegexMatcher ESLyricMatcher(ESLyricPattern, Line);

	TArray<FDreamMusicLyricTimestamp> Timestamps;
	TArray<FString> Contents;

	while (ESLyricMatcher.FindNext())
	{
		int32 Minutes = FCString::Atoi(*ESLyricMatcher.GetCaptureGroup(1));
		int32 Seconds = FCString::Atoi(*ESLyricMatcher.GetCaptureGroup(2));
		FString MillisecondsStr = ESLyricMatcher.GetCaptureGroup(3);
		FString WordContent = ESLyricMatcher.GetCaptureGroup(4);

		int32 Milliseconds = (MillisecondsStr.Len() == 2) ? FCString::Atoi(*MillisecondsStr) * 10 : FCString::Atoi(*MillisecondsStr);
		FDreamMusicLyricTimestamp Timestamp(Minutes, Seconds, Milliseconds);

		Timestamps.Add(Timestamp);
		Contents.Add(WordContent);
	}

	if (Timestamps.Num() == 0)
	{
		return;
	}

	// Build words with proper timing
	TArray<FDreamMusicLyricWord> Words;
	FString FullContent;

	for (int32 i = 0; i < Timestamps.Num(); i++)
	{
		FDreamMusicLyricTimestamp StartTimestamp = Timestamps[i];
		FString WordContent = Contents[i];

		for (int32 CharIndex = 0; CharIndex < WordContent.Len(); CharIndex++)
		{
			FString Char = WordContent.Mid(CharIndex, 1);

			if (Char == TEXT(" "))
			{
				FullContent += Char;
				continue;
			}

			FDreamMusicLyricTimestamp EndTimestamp;

			if (CharIndex == WordContent.Len() - 1)
			{
				if (i + 1 < Timestamps.Num())
				{
					EndTimestamp = Timestamps[i + 1];
				}
				else
				{
					int32 EndMs = StartTimestamp.TotalMilliseconds() + 500;
					EndTimestamp = FDreamMusicLyricTimestamp(
						EndMs / 3600000,
						(EndMs / 60000) % 60,
						(EndMs / 1000) % 60,
						EndMs % 1000
					);
				}
			}
			else
			{
				int32 CharDuration = 200;
				if (i + 1 < Timestamps.Num())
				{
					int32 TotalDuration = Timestamps[i + 1].TotalMilliseconds() - StartTimestamp.TotalMilliseconds();
					int32 RemainingChars = WordContent.Len() - CharIndex;
					CharDuration = FMath::Max(100, TotalDuration / RemainingChars);
				}

				int32 EndMs = StartTimestamp.TotalMilliseconds() + CharDuration;
				EndTimestamp = FDreamMusicLyricTimestamp(
					EndMs / 3600000,
					(EndMs / 60000) % 60,
					(EndMs / 1000) % 60,
					EndMs % 1000
				);

				if (CharIndex < WordContent.Len() - 1)
				{
					StartTimestamp = EndTimestamp;
				}
			}

			FDreamMusicLyricWord Word(StartTimestamp, EndTimestamp, Char);
			Words.Add(Word);
			FullContent += Char;
		}
	}

	// Assign content to the specified field
	if (TargetField == TEXT("Content"))
	{
		Lyric.Content = FullContent;
		Lyric.WordTimings = Words;
	}
	else if (TargetField == TEXT("Romanization"))
	{
		Lyric.Romanization = FullContent;
		Lyric.RomanizationWordTimings = Words;
	}
	else if (TargetField == TEXT("Translate"))
	{
		Lyric.Translate = FullContent;
		// Translation typically doesn't need word-by-word timing
	}
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

void FDreamMusicPlayerLyricFileParser_LRC::ProcessWordByWordContentForSpecificField(FDreamMusicLyric& Lyric, const FString& Line, const FString& TargetField)
{
	// Extract word timings using WordByWord format
	FRegexPattern WordPattern(TEXT("\\[(\\d{2}):(\\d{2})[.:](\\d{2,3})\\]([^\\[]*)"));
	FRegexMatcher WordMatcher(WordPattern, Line);

	TArray<FDreamMusicLyricTimestamp> Timestamps;
	TArray<FString> Contents;

	while (WordMatcher.FindNext())
	{
		int32 Minutes = FCString::Atoi(*WordMatcher.GetCaptureGroup(1));
		int32 Seconds = FCString::Atoi(*WordMatcher.GetCaptureGroup(2));
		FString MillisecondsStr = WordMatcher.GetCaptureGroup(3);
		FString WordContent = WordMatcher.GetCaptureGroup(4);

		int32 Milliseconds = (MillisecondsStr.Len() == 2) ? FCString::Atoi(*MillisecondsStr) * 10 : FCString::Atoi(*MillisecondsStr);
		FDreamMusicLyricTimestamp Timestamp(Minutes, Seconds, Milliseconds);

		Timestamps.Add(Timestamp);
		Contents.Add(WordContent);
	}

	if (Timestamps.Num() == 0)
	{
		// No WordByWord format found, try to extract content using regular LRC format
		FString ExtractedContent = ExtractContentFromLine(Line);
		if (TargetField == TEXT("Content"))
		{
			Lyric.Content = ExtractedContent;
		}
		else if (TargetField == TEXT("Romanization"))
		{
			Lyric.Romanization = ExtractedContent;
		}
		else if (TargetField == TEXT("Translate"))
		{
			Lyric.Translate = ExtractedContent;
		}
		return;
	}

	// Build words with proper timing
	TArray<FDreamMusicLyricWord> Words;
	FString FullContent;

	for (int32 i = 0; i < Timestamps.Num(); i++)
	{
		FDreamMusicLyricTimestamp CurrentTimestamp = Timestamps[i];
		FString WordContent = Contents[i].TrimStartAndEnd();

		// Skip empty content
		if (WordContent.IsEmpty())
		{
			continue;
		}

		// Process each character in the content
		for (int32 CharIndex = 0; CharIndex < WordContent.Len(); CharIndex++)
		{
			FString Char = WordContent.Mid(CharIndex, 1);

			// Handle spaces
			if (Char == TEXT(" "))
			{
				FullContent += Char;
				continue;
			}

			// Calculate end timestamp for this character
			FDreamMusicLyricTimestamp EndTimestamp;

			// If this is the last character in this content segment
			if (CharIndex == WordContent.Len() - 1)
			{
				// Use next timestamp if available
				if (i + 1 < Timestamps.Num())
				{
					EndTimestamp = Timestamps[i + 1];
				}
				else
				{
					// Default duration for last character
					int32 EndMs = CurrentTimestamp.TotalMilliseconds() + 500;
					EndTimestamp = FDreamMusicLyricTimestamp(
						EndMs / 3600000,
						(EndMs / 60000) % 60,
						(EndMs / 1000) % 60,
						EndMs % 1000
					);
				}
			}
			else
			{
				// For characters within the same segment, distribute time
				int32 CharDuration = 200; // Default 200ms per character
				if (i + 1 < Timestamps.Num())
				{
					int32 TotalDuration = Timestamps[i + 1].TotalMilliseconds() - CurrentTimestamp.TotalMilliseconds();
					int32 RemainingChars = WordContent.Len() - CharIndex;
					if (RemainingChars > 0)
					{
						CharDuration = FMath::Max(100, TotalDuration / RemainingChars);
					}
				}

				int32 EndMs = CurrentTimestamp.TotalMilliseconds() + CharDuration;
				EndTimestamp = FDreamMusicLyricTimestamp(
					EndMs / 3600000,
					(EndMs / 60000) % 60,
					(EndMs / 1000) % 60,
					EndMs % 1000
				);

				// Update timestamp for next character
				CurrentTimestamp = EndTimestamp;
			}

			FDreamMusicLyricWord Word(CurrentTimestamp, EndTimestamp, Char);
			Words.Add(Word);
			FullContent += Char;
		}
	}

	// Assign content to the specified field
	if (TargetField == TEXT("Content"))
	{
		Lyric.Content = FullContent;
		Lyric.WordTimings = Words;
	}
	else if (TargetField == TEXT("Romanization"))
	{
		Lyric.Romanization = FullContent;
		Lyric.RomanizationWordTimings = Words;
	}
	else if (TargetField == TEXT("Translate"))
	{
		Lyric.Translate = FullContent;
		// Translation typically doesn't need word-by-word timing
	}
}
